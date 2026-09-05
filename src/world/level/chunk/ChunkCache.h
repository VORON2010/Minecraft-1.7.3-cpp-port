#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "world/level/chunk/ChunkSource.h"
#include "world/level/chunk/storage/ChunkStorage.h"

#include "util/Memory.h"

class Level;

class ChunkCache : public ChunkSource
{
private:
	std::shared_ptr<LevelChunk> emptyChunk;

	std::unique_ptr<ChunkSource> source;
	std::unique_ptr<ChunkStorage> storage;
	std::unordered_map<uint_t, std::shared_ptr<LevelChunk>> chunkMap;
	std::vector<std::shared_ptr<LevelChunk>> chunks;

	Level &level;

public:
	ChunkCache(Level &level, ChunkStorage *storage, ChunkSource *source);

	bool hasChunk(int_t x, int_t z) override;
	std::shared_ptr<LevelChunk> getChunk(int_t x, int_t z) override;

	std::shared_ptr<LevelChunk> load(int_t x, int_t z);
	void saveEntities(LevelChunk &chunk);
	void save(LevelChunk &chunk);

	void postProcess(ChunkSource &parent, int_t x, int_t z) override;
	bool save(bool force, std::shared_ptr<ProgressListener> progressListener) override;
	bool tick() override;
	bool shouldSave() override;
	jstring gatherStats() override;
};
