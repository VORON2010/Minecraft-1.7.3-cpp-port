#include "world/level/SaveConverterMcRegion.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include "gzip/decompress.hpp"
#include "zlib.h"

#include "java/File.h"
#include "java/IOUtil.h"
#include "java/String.h"
#include "nbt/CompoundTag.h"
#include "nbt/NbtIo.h"
#include "util/ProgressListener.h"
#include "world/level/chunk/storage/RegionFile.h"
#include "world/level/chunk/storage/RegionFileCache.h"

namespace
{

constexpr int_t MCREGION_VERSION = 19132;

const std::regex CHUNK_FILE_PATTERN("^c\\.(-?[0-9a-z]+)\\.(-?[0-9a-z]+)\\.dat$");
const std::regex CHUNK_FOLDER_PATTERN("^[0-9a-z]{1,2}$");

struct ChunkFile
{
	jstring path;
	int_t x;
	int_t z;
};

std::shared_ptr<CompoundTag> readLevelDataFile(File &worldDirectory, const jstring &fileName)
{
	std::unique_ptr<File> file(File::open(worldDirectory, fileName));
	if (!file->exists())
		return nullptr;

	std::unique_ptr<std::istream> input(file->toStreamIn());
	if (!input)
		return nullptr;

	try
	{
		std::unique_ptr<CompoundTag> root(NbtIo::readCompressed(*input));
		return root->getCompound(u"Data");
	}
	catch (const std::exception &)
	{
		return nullptr;
	}
}

std::shared_ptr<CompoundTag> readLevelData(File &worldDirectory)
{
	std::shared_ptr<CompoundTag> data = readLevelDataFile(worldDirectory, u"level.dat");
	if (data == nullptr)
		data = readLevelDataFile(worldDirectory, u"level.dat_old");
	return data;
}

void writeLevelData(File &worldDirectory, const std::shared_ptr<CompoundTag> &data)
{
	std::shared_ptr<CompoundTag> root = std::make_shared<CompoundTag>();
	root->put(u"Data", data);

	std::unique_ptr<File> fileNew(File::open(worldDirectory, u"level.dat_new"));
	std::unique_ptr<File> fileOld(File::open(worldDirectory, u"level.dat_old"));
	std::unique_ptr<File> fileDat(File::open(worldDirectory, u"level.dat"));

	std::unique_ptr<std::ostream> output(fileNew->toStreamOut());
	if (!output)
		throw std::runtime_error("Failed to open level.dat_new for writing");
	NbtIo::writeCompressed(*root, *output);
	output->flush();
	output.reset();

	if (fileOld->exists())
		fileOld->remove();
	fileDat->renameTo(*fileOld);
	if (fileDat->exists())
		fileDat->remove();
	fileNew->renameTo(*fileDat);
	if (fileNew->exists())
		fileNew->remove();
}

void scanDimension(File &directory, std::vector<ChunkFile> &chunks, std::vector<jstring> &folders)
{
	for (auto &first : directory.listFiles())
	{
		std::string firstName = String::toUTF8(first->getName());
		if (!first->isDirectory() || !std::regex_match(firstName, CHUNK_FOLDER_PATTERN))
			continue;

		folders.push_back(first->toString());
		for (auto &second : first->listFiles())
		{
			std::string secondName = String::toUTF8(second->getName());
			if (!second->isDirectory() || !std::regex_match(secondName, CHUNK_FOLDER_PATTERN))
				continue;

			for (auto &file : second->listFiles())
			{
				std::smatch match;
				std::string fileName = String::toUTF8(file->getName());
				if (!std::regex_match(fileName, match, CHUNK_FILE_PATTERN))
					continue;

				chunks.push_back({file->toString(), std::stoi(match[1].str(), nullptr, 36), std::stoi(match[2].str(), nullptr, 36)});
			}
		}
	}
}

void convertChunks(File &directory, std::vector<ChunkFile> &chunks, int_t converted, int_t total, ProgressListener &progress)
{
	std::stable_sort(chunks.begin(), chunks.end(), [](const ChunkFile &a, const ChunkFile &b) {
		int_t ax = a.x >> 5;
		int_t bx = b.x >> 5;
		if (ax != bx)
			return ax < bx;
		return (a.z >> 5) < (b.z >> 5);
	});

	std::string directoryPath = String::toUTF8(directory.toString());
	for (const ChunkFile &chunk : chunks)
	{
		std::shared_ptr<RegionFile> region = RegionFileCache::getRegionFile(directoryPath, chunk.x, chunk.z);
		if (!region->hasChunk(chunk.x & 31, chunk.z & 31))
		{
			try
			{
				std::unique_ptr<File> file(File::open(chunk.path));
				std::unique_ptr<std::istream> input(file->toStreamIn());
				if (!input)
					throw std::runtime_error("Failed to open old chunk file");
				std::vector<char> compressed = IOUtil::readAllBytes(*input);
				std::string data = gzip::decompress(compressed.data(), compressed.size());

				
				
				
				uLongf deflatedBound = compressBound(static_cast<uLong>(data.size()));
				std::vector<byte_t> deflated(deflatedBound);
				uLongf deflatedSize = deflatedBound;
				if (compress2(reinterpret_cast<Bytef *>(deflated.data()), &deflatedSize,
					reinterpret_cast<const Bytef *>(data.data()), static_cast<uLong>(data.size()),
					Z_DEFAULT_COMPRESSION) != Z_OK)
					throw std::runtime_error("Failed to compress converted chunk");

				region->writeChunkData(chunk.x & 31, chunk.z & 31,
					deflated.data(), static_cast<int_t>(deflatedSize));
			}
			catch (const std::exception &exception)
			{
				std::cerr << "Failed to convert old chunk " << String::toUTF8(chunk.path) << ": " << exception.what() << '\n';
			}
		}

		converted++;
		progress.progressStagePercentage(static_cast<int_t>(std::lround(100.0 * converted / total)));
	}

	RegionFileCache::clearCache();
}

void deleteRecursive(std::vector<std::unique_ptr<File>> &files)
{
	for (auto &file : files)
	{
		if (file->isDirectory())
		{
			auto children = file->listFiles();
			deleteRecursive(children);
		}
		file->remove();
	}
}

void deleteFolders(const std::vector<jstring> &folders, int_t converted, int_t total, ProgressListener &progress)
{
	for (const jstring &folderPath : folders)
	{
		std::unique_ptr<File> folder(File::open(folderPath));
		auto children = folder->listFiles();
		deleteRecursive(children);
		folder->remove();
		converted++;
		progress.progressStagePercentage(static_cast<int_t>(std::lround(100.0 * converted / total)));
	}
}

}

SaveConverterMcRegion::SaveConverterMcRegion(File &savesDirectory)
	: savesDirectory(savesDirectory.toString())
{
	if (!savesDirectory.exists())
		savesDirectory.mkdirs();
}

jstring SaveConverterMcRegion::getFormatName() const
{
	return u"Scaevolus' McRegion";
}

bool SaveConverterMcRegion::isOldMapFormat(const jstring &name) const
{
	std::unique_ptr<File> saves(File::open(savesDirectory));
	std::unique_ptr<File> world(File::open(*saves, name));
	std::shared_ptr<CompoundTag> data = readLevelData(*world);
	return data != nullptr && (!data->contains(u"version") || data->getInt(u"version") == 0);
}

bool SaveConverterMcRegion::convertMapFormat(const jstring &name, ProgressListener &progress)
{
	progress.progressStagePercentage(0);
	std::vector<ChunkFile> overworldChunks;
	std::vector<ChunkFile> netherChunks;
	std::vector<jstring> overworldFolders;
	std::vector<jstring> netherFolders;

	std::unique_ptr<File> saves(File::open(savesDirectory));
	std::unique_ptr<File> world(File::open(*saves, name));
	std::unique_ptr<File> nether(File::open(*world, u"DIM-1"));

	std::cout << "Scanning folders...\n";
	scanDimension(*world, overworldChunks, overworldFolders);
	if (nether->exists())
		scanDimension(*nether, netherChunks, netherFolders);

	int_t total = static_cast<int_t>(overworldChunks.size() + netherChunks.size() + overworldFolders.size() + netherFolders.size());
	std::cout << "Total conversion count is " << total << '\n';
	convertChunks(*world, overworldChunks, 0, total, progress);
	convertChunks(*nether, netherChunks, static_cast<int_t>(overworldChunks.size()), total, progress);

	std::shared_ptr<CompoundTag> data = readLevelData(*world);
	data->putInt(u"version", MCREGION_VERSION);
	writeLevelData(*world, data);

	deleteFolders(overworldFolders, static_cast<int_t>(overworldChunks.size() + netherChunks.size()), total, progress);
	if (nether->exists())
	{
		deleteFolders(netherFolders,
			static_cast<int_t>(overworldChunks.size() + netherChunks.size() + overworldFolders.size()), total, progress);
	}

	return true;
}
