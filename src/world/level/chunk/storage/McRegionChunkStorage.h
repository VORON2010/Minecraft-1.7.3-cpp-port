#pragma once

#include "world/level/chunk/storage/ChunkStorage.h"

#include <condition_variable>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

class File;

class McRegionChunkStorage : public ChunkStorage
{
private:
	
	
	
	struct PendingWrite
	{
		std::shared_ptr<const std::string> nbt;
		std::uint64_t sequence = 0;
		bool queued = false;
	};

	std::string baseDir;
	Level *sizeOwner = nullptr;

	std::mutex mutex;
	std::condition_variable wake;
	std::condition_variable drained;
	std::unordered_map<long_t, PendingWrite> pending;
	std::deque<long_t> queue;
	std::uint64_t nextSequence = 0;
	long_t sizeDelta = 0;
	bool writing = false;
	bool stopping = false;
	std::thread worker;

	static long_t chunkKey(int_t x, int_t z);
	void writeLoop();
	void writeChunk(int_t x, int_t z, const std::string &nbt);

public:
	McRegionChunkStorage(std::shared_ptr<File> dir, bool create);
	~McRegionChunkStorage() override;

	std::shared_ptr<LevelChunk> load(Level &level, int_t x, int_t z) override;
	void save(Level &level, LevelChunk &chunk) override;
	void saveEntities(Level &level, LevelChunk &chunk) override;
	void tick() override;
	void flush() override;
};
