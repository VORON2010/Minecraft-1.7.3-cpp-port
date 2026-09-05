#pragma once

#include <cstddef>
#include <memory>
#include <unordered_map>
#include <vector>

#include "world/level/chunk/ChunkSource.h"

class Level;

class MultiplayerChunkCache : public ChunkSource
{
private:
	struct ChunkKey
	{
		int_t x;
		int_t z;

		ChunkKey(int_t x, int_t z) : x(x), z(z) {}

		bool operator==(const ChunkKey &other) const
		{
			return x == other.x && z == other.z;
		}
	};

	struct ChunkKeyHash
	{
		std::size_t operator()(const ChunkKey &key) const;
	};

	std::shared_ptr<LevelChunk> emptyChunk;
	std::unordered_map<ChunkKey, std::shared_ptr<LevelChunk>, ChunkKeyHash> chunkMapping;
	std::vector<std::shared_ptr<LevelChunk>> loadedChunks;
	Level &level;

public:
	explicit MultiplayerChunkCache(Level &level);

	bool hasChunk(int_t x, int_t z) override;
	std::shared_ptr<LevelChunk> create(int_t x, int_t z);
	void drop(int_t x, int_t z);
	std::shared_ptr<LevelChunk> getChunk(int_t x, int_t z) override;

	void postProcess(ChunkSource &parent, int_t x, int_t z) override;
	bool save(bool force, std::shared_ptr<ProgressListener> progressListener) override;
	bool tick() override;
	bool shouldSave() override;
	jstring gatherStats() override;
};
