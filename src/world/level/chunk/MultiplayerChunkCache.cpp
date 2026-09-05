#include "world/level/chunk/MultiplayerChunkCache.h"

#include <algorithm>

#include "world/level/Level.h"
#include "world/level/chunk/EmptyLevelChunk.h"
#include "world/level/chunk/LevelChunk.h"

#include "java/String.h"

#include "util/Memory.h"

std::size_t MultiplayerChunkCache::ChunkKeyHash::operator()(const ChunkKey &key) const
{
	uint_t hash = key.x < 0 ? 0x80000000U : 0U;
	hash |= (static_cast<uint_t>(key.x) & 0x7FFFU) << 16;
	hash |= key.z < 0 ? 0x8000U : 0U;
	hash |= static_cast<uint_t>(key.z) & 0x7FFFU;
	return static_cast<std::size_t>(hash);
}

MultiplayerChunkCache::MultiplayerChunkCache(Level &level) : level(level)
{
	emptyChunk = Util::make_shared<EmptyLevelChunk>(level, 0, 0);
}

bool MultiplayerChunkCache::hasChunk(int_t x, int_t z)
{
	(void)x;
	(void)z;
	return true;
}

std::shared_ptr<LevelChunk> MultiplayerChunkCache::create(int_t x, int_t z)
{
	ChunkKey key(x, z);
	std::shared_ptr<LevelChunk> chunk = Util::make_shared<LevelChunk>(level, x, z);
	chunk->skyLight.data.fill(static_cast<byte_t>(-1));

	auto entry = chunkMapping.find(key);
	if (entry == chunkMapping.end())
		chunkMapping.emplace(key, chunk);
	else
		entry->second = chunk;

	chunk->loaded = true;
	return chunk;
}

void MultiplayerChunkCache::drop(int_t x, int_t z)
{
	std::shared_ptr<LevelChunk> chunk = getChunk(x, z);
	if (!chunk->isEmpty())
		chunk->unload();

	chunkMapping.erase(ChunkKey(x, z));
	auto entry = std::find(loadedChunks.begin(), loadedChunks.end(), chunk);
	if (entry != loadedChunks.end())
		loadedChunks.erase(entry);
}

std::shared_ptr<LevelChunk> MultiplayerChunkCache::getChunk(int_t x, int_t z)
{
	auto entry = chunkMapping.find(ChunkKey(x, z));
	return entry == chunkMapping.end() ? emptyChunk : entry->second;
}

void MultiplayerChunkCache::postProcess(ChunkSource &parent, int_t x, int_t z)
{
	(void)parent;
	(void)x;
	(void)z;
}

bool MultiplayerChunkCache::save(bool force, std::shared_ptr<ProgressListener> progressListener)
{
	(void)force;
	(void)progressListener;
	return true;
}

bool MultiplayerChunkCache::tick()
{
	return false;
}

bool MultiplayerChunkCache::shouldSave()
{
	return false;
}

jstring MultiplayerChunkCache::gatherStats()
{
	return u"MultiplayerChunkCache: " + String::toString(static_cast<int_t>(chunkMapping.size()));
}
