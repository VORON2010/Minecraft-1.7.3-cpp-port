#include "world/level/chunk/ChunkCache.h"

#include "world/level/Level.h"

#include "world/level/chunk/EmptyLevelChunk.h"
#include "util/Profiler.h"

#include <iostream>
#include <stdexcept>

static uint_t chunkKey(int_t x, int_t z)
{
	uint_t key = x < 0 ? 0x80000000U : 0U;
	key |= (static_cast<uint_t>(x) & 0x7fffU) << 16;
	key |= z < 0 ? 0x8000U : 0U;
	return key | (static_cast<uint_t>(z) & 0x7fffU);
}

ChunkCache::ChunkCache(Level &level, ChunkStorage *storage, ChunkSource *source) : level(level)
{
	emptyChunk = Util::make_shared<EmptyLevelChunk>(level, 0, 0);

	this->source = std::unique_ptr<ChunkSource>(source);
	this->storage = std::unique_ptr<ChunkStorage>(storage);
}


bool ChunkCache::hasChunk(int_t x, int_t z)
{
	return chunkMap.find(chunkKey(x, z)) != chunkMap.end();
}

std::shared_ptr<LevelChunk> ChunkCache::getChunk(int_t x, int_t z)
{
	const uint_t key = chunkKey(x, z);
	auto entry = chunkMap.find(key);
	if (entry != chunkMap.end())
		return entry->second;

	std::shared_ptr<LevelChunk> chunk = load(x, z);
	if (chunk == nullptr)
		chunk = source == nullptr ? emptyChunk : source->getChunk(x, z);

	chunkMap.emplace(key, chunk);
	chunks.push_back(chunk);
	chunk->lightLava();
	chunk->load();

	if (!chunk->terrainPopulated && hasChunk(x + 1, z + 1) && hasChunk(x, z + 1) && hasChunk(x + 1, z))
		postProcess(*this, x, z);
	if (hasChunk(x - 1, z) && !getChunk(x - 1, z)->terrainPopulated && hasChunk(x - 1, z + 1) && hasChunk(x, z + 1) && hasChunk(x - 1, z))
		postProcess(*this, x - 1, z);
	if (hasChunk(x, z - 1) && !getChunk(x, z - 1)->terrainPopulated && hasChunk(x + 1, z - 1) && hasChunk(x, z - 1) && hasChunk(x + 1, z))
		postProcess(*this, x, z - 1);
	if (hasChunk(x - 1, z - 1) && !getChunk(x - 1, z - 1)->terrainPopulated && hasChunk(x - 1, z - 1) && hasChunk(x, z - 1) && hasChunk(x - 1, z))
		postProcess(*this, x - 1, z - 1);

	return chunk;
}

std::shared_ptr<LevelChunk> ChunkCache::load(int_t x, int_t z)
{
	if (storage == nullptr)
		return nullptr;
	Profiler::Scope ioProfile(Profiler::Section::IO);
	try
	{
		std::shared_ptr<LevelChunk> chunk = storage->load(level, x, z);
		if (chunk != nullptr)
			chunk->lastSaveTime = level.time;
		return chunk;
	}
	catch (const std::runtime_error &error)
	{
		std::cerr << error.what() << '\n';
		return nullptr;
	}
}

void ChunkCache::saveEntities(LevelChunk &chunk)
{
	if (storage == nullptr)
		return;
	Profiler::Scope ioProfile(Profiler::Section::IO);
	try
	{
		storage->saveEntities(level, chunk);
	}
	catch (const std::runtime_error &error)
	{
		std::cerr << error.what() << '\n';
	}
}

void ChunkCache::save(LevelChunk &chunk)
{
	if (storage == nullptr)
		return;
	Profiler::Scope ioProfile(Profiler::Section::IO);
	try
	{
		chunk.lastSaveTime = level.time;
		storage->save(level, chunk);
	}
	catch (const std::runtime_error &error)
	{
		std::cerr << error.what() << '\n';
	}
}

void ChunkCache::postProcess(ChunkSource &parent, int_t x, int_t z)
{
	std::shared_ptr<LevelChunk> chunk = getChunk(x, z);
	if (!chunk->terrainPopulated)
	{
		chunk->terrainPopulated = true;
		if (source != nullptr)
		{
			source->postProcess(parent, x, z);
			chunk->markUnsaved();
		}
	}
}

bool ChunkCache::save(bool force, std::shared_ptr<ProgressListener> progressListener)
{
	(void)progressListener;
	int_t throttle = 0;
	for (size_t i = 0; i < chunks.size(); ++i)
	{
		std::shared_ptr<LevelChunk> chunk = chunks[i];
		if (force && !chunk->dontSave)
			saveEntities(*chunk);
		if (chunk->shouldSave(force))
		{
			save(*chunk);
			chunk->unsaved = false;
			if (++throttle == 24 && !force)
				return false;
		}
	}
	if (force && storage != nullptr)
		storage->flush();
	return true;
}

bool ChunkCache::tick()
{
	
	if (storage != nullptr)
		storage->tick();
	if (source == nullptr)
		throw std::runtime_error("java.lang.NullPointerException");
	return source->tick();
}

bool ChunkCache::shouldSave()
{
	return true;
}

jstring ChunkCache::gatherStats()
{
	return u"ServerChunkCache: " + String::toString(static_cast<int_t>(chunkMap.size())) + u" Drop: 0";
}
