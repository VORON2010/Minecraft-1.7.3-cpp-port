#include "tools/SaveConverterSmoke.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "java/File.h"
#include "java/String.h"
#include "nbt/CompoundTag.h"
#include "nbt/NbtIo.h"
#include "util/ProgressListener.h"
#include "world/level/Level.h"
#include "world/level/SaveConverterMcRegion.h"
#include "world/level/chunk/storage/RegionFile.h"
#include "world/level/chunk/storage/RegionFileCache.h"

namespace
{

class CapturingProgressListener : public ProgressListener
{
public:
	std::vector<int_t> percentages;

	void progressStartNoAbort(const jstring &title) override { (void)title; }
	void progressStart(const jstring &title) override { (void)title; }
	void progressStage(const jstring &status) override { (void)status; }
	void progressStagePercentage(int_t percentage) override { percentages.push_back(percentage); }
};

bool expect(bool condition, const char *message)
{
	if (!condition)
		std::cerr << "FAILED: " << message << '\n';
	return condition;
}

void removeDirectory(File &directory)
{
	if (!directory.exists())
		return;
	auto children = directory.listFiles();
	Level::deleteRecursive(children);
	directory.remove();
}

void writeLevelData(File &world)
{
	std::shared_ptr<CompoundTag> data = std::make_shared<CompoundTag>();
	data->putLong(u"RandomSeed", 12345LL);
	data->putLong(u"LastPlayed", 67890LL);
	data->putString(u"LevelName", u"Alpha conversion smoke");

	CompoundTag root;
	root.put(u"Data", data);
	std::unique_ptr<File> levelDat(File::open(world, u"level.dat"));
	std::unique_ptr<std::ostream> output(levelDat->toStreamOut());
	NbtIo::writeCompressed(root, *output);
}

void writeOldChunk(File &dimension, int_t x, int_t z, const jstring &marker)
{
	std::unique_ptr<File> first(File::open(dimension, String::toString(x & 63, 36)));
	std::unique_ptr<File> second(File::open(*first, String::toString(z & 63, 36)));
	second->mkdirs();
	jstring fileName = u"c." + String::toString(x, 36) + u"." + String::toString(z, 36) + u".dat";
	std::unique_ptr<File> file(File::open(*second, fileName));

	std::shared_ptr<CompoundTag> level = std::make_shared<CompoundTag>();
	level->putInt(u"xPos", x);
	level->putInt(u"zPos", z);
	level->putString(u"Marker", marker);
	CompoundTag root;
	root.put(u"Level", level);
	std::unique_ptr<std::ostream> output(file->toStreamOut());
	NbtIo::writeCompressed(root, *output);
}

std::string makeChunkData(int_t x, int_t z, const jstring &marker)
{
	std::shared_ptr<CompoundTag> level = std::make_shared<CompoundTag>();
	level->putInt(u"xPos", x);
	level->putInt(u"zPos", z);
	level->putString(u"Marker", marker);
	CompoundTag root;
	root.put(u"Level", level);
	std::ostringstream output(std::ios::out | std::ios::binary);
	NbtIo::write(root, output);
	return output.str();
}

jstring readRegionMarker(File &dimension, int_t x, int_t z)
{
	std::shared_ptr<RegionFile> region = RegionFileCache::getRegionFile(String::toUTF8(dimension.toString()), x, z);
	std::vector<byte_t> data = region->getChunkData(x & 31, z & 31);
	if (data.empty())
		return jstring();
	std::string raw(reinterpret_cast<const char *>(data.data()), data.size());
	std::istringstream input(raw, std::ios::in | std::ios::binary);
	std::unique_ptr<CompoundTag> root(NbtIo::read(input));
	return root->getCompound(u"Level")->getString(u"Marker");
}

std::shared_ptr<CompoundTag> readLevelDataFile(File &world, const jstring &name)
{
	std::unique_ptr<File> file(File::open(world, name));
	std::unique_ptr<std::istream> input(file->toStreamIn());
	std::unique_ptr<CompoundTag> root(NbtIo::readCompressed(*input));
	return root->getCompound(u"Data");
}

}

int runSaveConverterSmoke()
{
	std::unique_ptr<File> root(File::open(u"build/save-converter-smoke"));
	RegionFileCache::clearCache();
	removeDirectory(*root);

	try
	{
		std::unique_ptr<File> saves(File::open(*root, u"saves"));
		std::unique_ptr<File> world(File::open(*saves, u"AlphaWorld"));
		std::unique_ptr<File> nether(File::open(*world, u"DIM-1"));
		std::unique_ptr<File> untouched(File::open(*world, u"keep"));
		nether->mkdirs();
		untouched->mkdirs();
		writeLevelData(*world);
		writeOldChunk(*world, 0, 0, u"overworld");
		writeOldChunk(*nether, -33, 65, u"nether");
		writeOldChunk(*world, 32, 0, u"legacy-collision");

		std::shared_ptr<RegionFile> existing = RegionFileCache::getRegionFile(String::toUTF8(world->toString()), 32, 0);
		std::string existingData = makeChunkData(32, 0, u"region-wins");
		existing->writeChunkData(0, 0, reinterpret_cast<const byte_t *>(existingData.data()), static_cast<int_t>(existingData.size()));
		RegionFileCache::clearCache();

		SaveConverterMcRegion converter(*saves);
		CapturingProgressListener progress;
		bool ok = true;
		ok &= expect(converter.getFormatName() == u"Scaevolus' McRegion", "converter format name should match Beta");
		ok &= expect(converter.isOldMapFormat(u"AlphaWorld"), "versionless Alpha world should require conversion");
		ok &= expect(converter.convertMapFormat(u"AlphaWorld", progress), "converter should report success");
		ok &= expect(!converter.isOldMapFormat(u"AlphaWorld"), "converted world should no longer be old format");
		ok &= expect(!progress.percentages.empty() && progress.percentages.front() == 0 && progress.percentages.back() == 100,
			"conversion progress should run from zero to 100");

		std::shared_ptr<CompoundTag> convertedData = readLevelDataFile(*world, u"level.dat");
		std::shared_ptr<CompoundTag> backupData = readLevelDataFile(*world, u"level.dat_old");
		ok &= expect(convertedData->getInt(u"version") == 19132 && convertedData->getString(u"LevelName") == u"Alpha conversion smoke",
			"conversion should update only the save version in current level.dat");
		ok &= expect(!backupData->contains(u"version") && backupData->getString(u"LevelName") == u"Alpha conversion smoke",
			"conversion should retain the original level.dat as level.dat_old");
		ok &= expect(readRegionMarker(*world, 0, 0) == u"overworld", "overworld Alpha chunk should migrate to McRegion");
		ok &= expect(readRegionMarker(*nether, -33, 65) == u"nether", "negative Nether Alpha chunk should migrate to DIM-1 McRegion");
		ok &= expect(readRegionMarker(*world, 32, 0) == u"region-wins", "existing McRegion chunk should not be overwritten");
		RegionFileCache::clearCache();

		std::unique_ptr<File> oldOverworldFolder(File::open(*world, u"0"));
		std::unique_ptr<File> oldCollisionFolder(File::open(*world, u"w"));
		std::unique_ptr<File> oldNetherFolder(File::open(*nether, u"v"));
		ok &= expect(!oldOverworldFolder->exists() && !oldCollisionFolder->exists() && !oldNetherFolder->exists(),
			"converted Alpha hash folders should be deleted");
		ok &= expect(untouched->exists(), "non-Alpha world folder should be left untouched");

		removeDirectory(*root);
		if (!ok)
			return 1;
		std::cout << "Save converter smoke passed.\n";
		return 0;
	}
	catch (const std::exception &exception)
	{
		RegionFileCache::clearCache();
		removeDirectory(*root);
		std::cerr << "Save converter smoke exception: " << exception.what() << '\n';
		return 1;
	}
}
