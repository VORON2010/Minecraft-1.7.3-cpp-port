#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <mutex>

#include "java/Type.h"

class RegionFile
{
private:
	static const int SECTOR_BYTES = 4096;
	static const int SECTOR_INTS = 1024;

	std::string filePath;
	std::fstream dataFile;
	int_t offsets[1024] = {};
	int_t timestamps[1024] = {};
	std::vector<bool> sectorFree;
	int_t sizeDelta = 0;
	long_t lastModified = 0;

	std::recursive_mutex mutex;

	static const byte_t emptySector[4096];

	void writeInt(int_t offset, int_t value);
	int_t readInt();
	void writeRawInt(int_t value);

	bool outOfBounds(int_t x, int_t z);
	int_t getOffset(int_t x, int_t z);
	void setOffset(int_t x, int_t z, int_t val);
	void setTimestamp(int_t x, int_t z, int_t val);
	void writeSectors(int_t sectorNumber, const byte_t *data, int_t length);

public:
	RegionFile(const std::string &path);
	~RegionFile();

	
	std::vector<byte_t> getChunkData(int_t x, int_t z);

	
	void writeChunkData(int_t x, int_t z, const byte_t *data, int_t length);

	bool hasChunk(int_t x, int_t z);

	int_t getSizeDelta();

	void close();
};
