#include "world/level/chunk/storage/RegionFileCache.h"

#include "world/level/chunk/storage/RegionFile.h"

#include "java/File.h"
#include "java/String.h"

#include <iostream>
#include <sstream>

std::unordered_map<std::string, std::shared_ptr<RegionFile>> RegionFileCache::cache;
std::recursive_mutex RegionFileCache::mutex;


static int_t regionCoord(int_t chunkCoord)
{
	return chunkCoord >> 5;
}

std::string RegionFileCache::getRegionPath(const std::string &baseDir, int_t chunkX, int_t chunkZ)
{
	std::ostringstream oss;
	oss << baseDir << "/region/r." << regionCoord(chunkX) << "." << regionCoord(chunkZ) << ".mcr";
	return oss.str();
}

std::shared_ptr<RegionFile> RegionFileCache::getRegionFile(const std::string &baseDir, int_t chunkX, int_t chunkZ)
{
	std::lock_guard<std::recursive_mutex> lock(mutex);

	std::string path = getRegionPath(baseDir, chunkX, chunkZ);

	auto it = cache.find(path);
	if (it != cache.end())
		return it->second;

	
	jstring regionDirPath = String::fromUTF8(baseDir) + u"/region";
	std::unique_ptr<File> regionDir(File::open(regionDirPath));
	if (!regionDir->exists())
		regionDir->mkdirs();

	if (cache.size() >= MAX_CACHE_SIZE)
		clearCache();

	auto regionFile = std::make_shared<RegionFile>(path);
	cache[path] = regionFile;
	return regionFile;
}

void RegionFileCache::clearCache()
{
	std::lock_guard<std::recursive_mutex> lock(mutex);

	for (auto &pair : cache)
		pair.second->close();

	cache.clear();
}

int_t RegionFileCache::getSizeDelta(const std::string &baseDir, int_t chunkX, int_t chunkZ)
{
	auto regionFile = getRegionFile(baseDir, chunkX, chunkZ);
	return regionFile->getSizeDelta();
}
