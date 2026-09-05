#include "world/level/chunk/storage/RegionFile.h"

#include <cstring>
#include <iostream>
#include <algorithm>
#include <chrono>

#include "zlib.h"

const byte_t RegionFile::emptySector[4096] = {};


static void writeBE32(std::fstream &f, int_t value)
{
	byte_t buf[4];
	buf[0] = (value >> 24) & 0xFF;
	buf[1] = (value >> 16) & 0xFF;
	buf[2] = (value >> 8) & 0xFF;
	buf[3] = value & 0xFF;
	f.write(reinterpret_cast<char *>(buf), 4);
}

static int_t readBE32(std::fstream &f)
{
	byte_t buf[4];
	f.read(reinterpret_cast<char *>(buf), 4);
	return (static_cast<int_t>(buf[0] & 0xFF) << 24) |
	       (static_cast<int_t>(buf[1] & 0xFF) << 16) |
	       (static_cast<int_t>(buf[2] & 0xFF) << 8) |
	       static_cast<int_t>(buf[3] & 0xFF);
}

RegionFile::RegionFile(const std::string &path)
{
	filePath = path;
	sizeDelta = 0;

	
	dataFile.open(path, std::ios::in | std::ios::out | std::ios::binary);
	if (!dataFile.is_open())
	{
		
		dataFile.open(path, std::ios::out | std::ios::binary);
		dataFile.close();
		dataFile.open(path, std::ios::in | std::ios::out | std::ios::binary);
	}

	if (!dataFile.is_open())
	{
		std::cerr << "Failed to open region file: " << path << std::endl;
		return;
	}

	
	dataFile.seekg(0, std::ios::end);
	long_t fileLength = static_cast<long_t>(dataFile.tellg());

	
	if (fileLength < 4096L)
	{
		dataFile.seekp(0, std::ios::beg);
		for (int_t i = 0; i < 1024; ++i)
			writeBE32(dataFile, 0);
		for (int_t i = 0; i < 1024; ++i)
			writeBE32(dataFile, 0);

		sizeDelta += 8192;
		dataFile.flush();

		dataFile.seekg(0, std::ios::end);
		fileLength = static_cast<long_t>(dataFile.tellg());
	}

	
	if ((fileLength & 4095L) != 0L)
	{
		dataFile.seekp(0, std::ios::end);
		for (long_t i = 0; i < (fileLength & 4095L); ++i)
			dataFile.put(0);
		dataFile.flush();

		dataFile.seekg(0, std::ios::end);
		fileLength = static_cast<long_t>(dataFile.tellg());
	}

	
	int_t totalSectors = static_cast<int_t>(fileLength / 4096);
	sectorFree.resize(totalSectors, true);

	
	sectorFree[0] = false;
	if (totalSectors > 1)
		sectorFree[1] = false;

	
	dataFile.seekg(0, std::ios::beg);
	for (int_t i = 0; i < 1024; ++i)
	{
		int_t offset = readBE32(dataFile);
		offsets[i] = offset;
		if (offset != 0)
		{
			int_t sectorStart = offset >> 8;
			int_t sectorCount = offset & 255;
			if (sectorStart + sectorCount <= static_cast<int_t>(sectorFree.size()))
			{
				for (int_t s = 0; s < sectorCount; ++s)
					sectorFree[sectorStart + s] = false;
			}
		}
	}

	
	for (int_t i = 0; i < 1024; ++i)
		timestamps[i] = readBE32(dataFile);
}

RegionFile::~RegionFile()
{
	close();
}

void RegionFile::close()
{
	std::lock_guard<std::recursive_mutex> lock(mutex);
	if (dataFile.is_open())
	{
		dataFile.flush();
		dataFile.close();
	}
}

int_t RegionFile::getSizeDelta()
{
	std::lock_guard<std::recursive_mutex> lock(mutex);
	int_t delta = sizeDelta;
	sizeDelta = 0;
	return delta;
}

bool RegionFile::outOfBounds(int_t x, int_t z)
{
	return x < 0 || x >= 32 || z < 0 || z >= 32;
}

int_t RegionFile::getOffset(int_t x, int_t z)
{
	return offsets[x + z * 32];
}

bool RegionFile::hasChunk(int_t x, int_t z)
{
	std::lock_guard<std::recursive_mutex> lock(mutex);
	if (outOfBounds(x, z))
		return false;
	return getOffset(x, z) != 0;
}

std::vector<byte_t> RegionFile::getChunkData(int_t x, int_t z)
{
	std::lock_guard<std::recursive_mutex> lock(mutex);

	if (outOfBounds(x, z))
		return {};

	int_t offset = getOffset(x, z);
	if (offset == 0)
		return {};

	int_t sectorNumber = offset >> 8;
	int_t numSectors = offset & 255;

	if (sectorNumber + numSectors > static_cast<int_t>(sectorFree.size()))
		return {};

	dataFile.seekg(static_cast<long_t>(sectorNumber) * 4096);

	int_t length = readBE32(dataFile);
	if (length > 4096 * numSectors || length <= 0)
		return {};

	byte_t compressionType;
	dataFile.read(reinterpret_cast<char *>(&compressionType), 1);

	int_t dataLength = length - 1;
	std::vector<byte_t> compressedData(dataLength);
	dataFile.read(reinterpret_cast<char *>(compressedData.data()), dataLength);

	
	if (compressionType == 1)
	{
		
		z_stream strm = {};
		strm.next_in = reinterpret_cast<Bytef *>(compressedData.data());
		strm.avail_in = static_cast<uInt>(dataLength);

		if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK)
			return {};

		std::vector<byte_t> result;
		byte_t outbuf[16384];
		int ret;
		do
		{
			strm.next_out = reinterpret_cast<Bytef *>(outbuf);
			strm.avail_out = sizeof(outbuf);
			ret = inflate(&strm, Z_NO_FLUSH);
			if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR)
			{
				inflateEnd(&strm);
				return {};
			}
			size_t have = sizeof(outbuf) - strm.avail_out;
			result.insert(result.end(), outbuf, outbuf + have);
		} while (ret != Z_STREAM_END);

		inflateEnd(&strm);
		return result;
	}
	else if (compressionType == 2)
	{
		
		z_stream strm = {};
		strm.next_in = reinterpret_cast<Bytef *>(compressedData.data());
		strm.avail_in = static_cast<uInt>(dataLength);

		if (inflateInit(&strm) != Z_OK)
			return {};

		std::vector<byte_t> result;
		byte_t outbuf[16384];
		int ret;
		do
		{
			strm.next_out = reinterpret_cast<Bytef *>(outbuf);
			strm.avail_out = sizeof(outbuf);
			ret = inflate(&strm, Z_NO_FLUSH);
			if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR)
			{
				inflateEnd(&strm);
				return {};
			}
			size_t have = sizeof(outbuf) - strm.avail_out;
			result.insert(result.end(), outbuf, outbuf + have);
		} while (ret != Z_STREAM_END);

		inflateEnd(&strm);
		return result;
	}

	return {};
}

void RegionFile::writeChunkData(int_t x, int_t z, const byte_t *data, int_t length)
{
	std::lock_guard<std::recursive_mutex> lock(mutex);

	if (outOfBounds(x, z))
		return;

	int_t offset = getOffset(x, z);
	int_t sectorNumber = offset >> 8;
	int_t existingSectors = offset & 255;

	
	int_t neededSectors = (length + 5) / 4096 + 1;

	if (neededSectors >= 256)
		return;

	if (sectorNumber != 0 && existingSectors == neededSectors)
	{
		
		writeSectors(sectorNumber, data, length);
	}
	else
	{
		
		for (int_t i = 0; i < existingSectors; ++i)
		{
			if (sectorNumber + i < static_cast<int_t>(sectorFree.size()))
				sectorFree[sectorNumber + i] = true;
		}

		
		int_t runStart = -1;
		int_t runLength = 0;

		for (int_t i = 2; i < static_cast<int_t>(sectorFree.size()); ++i)
		{
			if (sectorFree[i])
			{
				if (runLength == 0)
					runStart = i;
				++runLength;
				if (runLength >= neededSectors)
					break;
			}
			else
			{
				runLength = 0;
			}
		}

		if (runLength >= neededSectors)
		{
			
			sectorNumber = runStart;
			setOffset(x, z, runStart << 8 | neededSectors);

			for (int_t i = 0; i < neededSectors; ++i)
				sectorFree[sectorNumber + i] = false;

			writeSectors(sectorNumber, data, length);
		}
		else
		{
			
			dataFile.seekp(0, std::ios::end);
			sectorNumber = static_cast<int_t>(sectorFree.size());

			for (int_t i = 0; i < neededSectors; ++i)
			{
				dataFile.write(reinterpret_cast<const char *>(emptySector), 4096);
				sectorFree.push_back(false);
			}

			sizeDelta += 4096 * neededSectors;
			writeSectors(sectorNumber, data, length);
			setOffset(x, z, sectorNumber << 8 | neededSectors);
		}
	}

	
	auto now = std::chrono::system_clock::now();
	int_t timestamp = static_cast<int_t>(std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count());
	setTimestamp(x, z, timestamp);

	dataFile.flush();
}

void RegionFile::writeSectors(int_t sectorNumber, const byte_t *data, int_t length)
{
	dataFile.seekp(static_cast<long_t>(sectorNumber) * 4096);

	
	byte_t header[5];
	int_t totalLen = length + 1;
	header[0] = (totalLen >> 24) & 0xFF;
	header[1] = (totalLen >> 16) & 0xFF;
	header[2] = (totalLen >> 8) & 0xFF;
	header[3] = totalLen & 0xFF;
	header[4] = 2; 
	dataFile.write(reinterpret_cast<char *>(header), 5);
	dataFile.write(reinterpret_cast<const char *>(data), length);
}

void RegionFile::setOffset(int_t x, int_t z, int_t val)
{
	offsets[x + z * 32] = val;
	dataFile.seekp(static_cast<long_t>((x + z * 32)) * 4);
	writeBE32(dataFile, val);
}

void RegionFile::setTimestamp(int_t x, int_t z, int_t val)
{
	timestamps[x + z * 32] = val;
	dataFile.seekp(4096 + static_cast<long_t>((x + z * 32)) * 4);
	writeBE32(dataFile, val);
}
