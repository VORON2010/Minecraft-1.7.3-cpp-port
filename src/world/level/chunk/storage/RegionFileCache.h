#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>

#include "java/Type.h"

class File;
class RegionFile;

class RegionFileCache
{
private:
	static const int MAX_CACHE_SIZE = 256;

	static std::unordered_map<std::string, std::shared_ptr<RegionFile>> cache;
	static std::recursive_mutex mutex;

	static std::string getRegionPath(const std::string &baseDir, int_t chunkX, int_t chunkZ);

public:
	static std::shared_ptr<RegionFile> getRegionFile(const std::string &baseDir, int_t chunkX, int_t chunkZ);
	static void clearCache();

	static int_t getSizeDelta(const std::string &baseDir, int_t chunkX, int_t chunkZ);
};
