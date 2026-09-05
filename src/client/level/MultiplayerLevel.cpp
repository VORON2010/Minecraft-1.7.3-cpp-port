#include "client/level/MultiplayerLevel.h"

#include <cstring>
#include <algorithm>

#include "network/NetClientHandler.h"
#include "network/PacketCore.h"
#include "world/level/LevelListener.h"
#include "world/level/chunk/MultiplayerChunkCache.h"

#include "util/Memory.h"

namespace
{

int_t javaIntAdd(int_t left, int_t right)
{
	uint_t resultBits = static_cast<uint_t>(left) + static_cast<uint_t>(right);
	int_t result;
	std::memcpy(&result, &resultBits, sizeof(result));
	return result;
}

int_t javaIntMultiply(int_t left, int_t right)
{
	uint_t resultBits = static_cast<uint_t>(left) * static_cast<uint_t>(right);
	int_t result;
	std::memcpy(&result, &resultBits, sizeof(result));
	return result;
}

int_t javaIntNegate(int_t value)
{
	uint_t resultBits = 0U - static_cast<uint_t>(value);
	int_t result;
	std::memcpy(&result, &resultBits, sizeof(result));
	return result;
}

long_t javaLongIncrement(long_t value)
{
	ulong_t resultBits = static_cast<ulong_t>(value) + 1ULL;
	long_t result;
	std::memcpy(&result, &resultBits, sizeof(result));
	return result;
}

}

MultiplayerLevel::MultiplayerLevel(NetClientHandler &networkHandler, long_t seed, int_t dimension)
	: Level(u"MpServer", dimension, seed, false), networkHandler(&networkHandler)
{
	multiplayerChunkCache = Util::make_shared<MultiplayerChunkCache>(*this);
	setChunkSource(multiplayerChunkCache);
	xSpawn = 8;
	ySpawn = 64;
	zSpawn = 8;
	isOnline = true;
}

void MultiplayerLevel::setSpawnLocation()
{
	xSpawn = 8;
	ySpawn = 64;
	zSpawn = 8;
}

void MultiplayerLevel::tick()
{
	time = javaLongIncrement(time);
	int_t newDarken = getSkyDarken(1.0f);
	if (newDarken != skyDarken)
	{
		skyDarken = newDarken;
		
		
		
		for (LevelListener *listener : listeners)
			listener->skyColorChanged();
	}

	for (int_t i = 0; i < 10 && !pendingEntities.empty(); ++i)
	{
		std::shared_ptr<Entity> entity = *pendingEntities.begin();
		if (std::none_of(entities.begin(), entities.end(), [&](const std::shared_ptr<Entity> &candidate) {
			return candidate->entityId == entity->entityId;
		}))
			addEntity(entity);
	}

	
	
	std::shared_ptr<MultiplayerLevel> keepAlive;
	if (networkHandler != nullptr)
	{
		keepAlive = networkHandler->getLevel();
		networkHandler->processReadPackets();
	}

	for (auto it = pendingBlockChanges.begin(); it != pendingBlockChanges.end();)
	{
		--it->ticks;
		if (it->ticks == 0)
		{
			Level::setTileAndDataNoUpdate(it->x, it->y, it->z, it->tile, it->data);
			setTileDirty(it->x, it->y, it->z);
			it = pendingBlockChanges.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void MultiplayerLevel::scheduleBlockUpdate(int_t x, int_t y, int_t z, int_t tileId, int_t delay)
{
	(void)x;
	(void)y;
	(void)z;
	(void)tileId;
	(void)delay;
}

bool MultiplayerLevel::tickPendingTicks(bool unknown)
{
	(void)unknown;
	return false;
}

void MultiplayerLevel::doPreChunk(int_t x, int_t z, bool load)
{
	if (load)
		multiplayerChunkCache->create(x, z);
	else
		multiplayerChunkCache->drop(x, z);

	if (!load)
	{
		int_t x0 = javaIntMultiply(x, 16);
		int_t z0 = javaIntMultiply(z, 16);
		setTilesDirty(x0, 0, z0, javaIntAdd(x0, 15), 128, javaIntAdd(z0, 15));
	}
}

MultiplayerChunkCache &MultiplayerLevel::getMultiplayerChunkCache()
{
	return *multiplayerChunkCache;
}

bool MultiplayerLevel::addEntity(std::shared_ptr<Entity> entity)
{
	bool added = Level::addEntity(entity);
	trackedEntities.emplace(entity);
	if (!added)
		pendingEntities.emplace(entity);
	return added;
}

void MultiplayerLevel::removeEntity(std::shared_ptr<Entity> entity)
{
	Level::removeEntity(entity);
	trackedEntities.erase(entity);
}

void MultiplayerLevel::entityAdded(std::shared_ptr<Entity> entity)
{
	Level::entityAdded(entity);
	pendingEntities.erase(entity);
}

void MultiplayerLevel::entityRemoved(std::shared_ptr<Entity> entity)
{
	Level::entityRemoved(entity);
	if (trackedEntities.find(entity) != trackedEntities.end())
		pendingEntities.emplace(entity);
}

void MultiplayerLevel::addEntityById(int_t entityId, std::shared_ptr<Entity> entity)
{
	std::shared_ptr<Entity> existing = getEntityById(entityId);
	if (existing != nullptr)
		removeEntity(existing);

	trackedEntities.emplace(entity);
	entity->entityId = entityId;
	if (!addEntity(entity))
		pendingEntities.emplace(entity);
	entitiesById[entityId] = std::move(entity);
}

std::shared_ptr<Entity> MultiplayerLevel::getEntityById(int_t entityId) const
{
	auto found = entitiesById.find(entityId);
	return found == entitiesById.end() ? nullptr : found->second;
}

std::shared_ptr<Entity> MultiplayerLevel::removeEntityById(int_t entityId)
{
	auto found = entitiesById.find(entityId);
	if (found == entitiesById.end())
		return nullptr;

	std::shared_ptr<Entity> entity = found->second;
	entitiesById.erase(found);
	trackedEntities.erase(entity);
	removeEntity(entity);
	return entity;
}

void MultiplayerLevel::addPendingBlockChange(int_t x, int_t y, int_t z, int_t tile, int_t data)
{
	pendingBlockChanges.emplace_back(x, y, z, tile, data);
}

bool MultiplayerLevel::setDataNoUpdate(int_t x, int_t y, int_t z, int_t data)
{
	int_t oldTile = getTile(x, y, z);
	int_t oldData = getData(x, y, z);
	if (!Level::setDataNoUpdate(x, y, z, data))
		return false;
	addPendingBlockChange(x, y, z, oldTile, oldData);
	return true;
}

bool MultiplayerLevel::setTileAndDataNoUpdate(int_t x, int_t y, int_t z, int_t tile, int_t data)
{
	int_t oldTile = getTile(x, y, z);
	int_t oldData = getData(x, y, z);
	if (!Level::setTileAndDataNoUpdate(x, y, z, tile, data))
		return false;
	addPendingBlockChange(x, y, z, oldTile, oldData);
	return true;
}

bool MultiplayerLevel::setTileNoUpdate(int_t x, int_t y, int_t z, int_t tile)
{
	int_t oldTile = getTile(x, y, z);
	int_t oldData = getData(x, y, z);
	if (!Level::setTileNoUpdate(x, y, z, tile))
		return false;
	addPendingBlockChange(x, y, z, oldTile, oldData);
	return true;
}

bool MultiplayerLevel::setTileAndDataRemote(int_t x, int_t y, int_t z, int_t tile, int_t data)
{
	clearPendingBlockChanges(x, y, z, x, y, z);
	if (!Level::setTileAndDataNoUpdate(x, y, z, tile, data))
		return false;
	notifyBlockChange(x, y, z, tile);
	return true;
}

void MultiplayerLevel::setChunkData(int_t x, int_t y, int_t z,
	int_t xSize, int_t ySize, int_t zSize, const std::vector<byte_t> &data)
{
	int_t chunkX0 = x >> 4;
	int_t chunkZ0 = z >> 4;
	int_t chunkX1 = javaIntAdd(javaIntAdd(x, xSize), -1) >> 4;
	int_t chunkZ1 = javaIntAdd(javaIntAdd(z, zSize), -1) >> 4;
	int_t offset = 0;
	int_t y0 = y < 0 ? 0 : y;
	int_t y1 = javaIntAdd(y, ySize);
	if (y1 > 128)
		y1 = 128;

	for (int_t chunkX = chunkX0; chunkX <= chunkX1; ++chunkX)
	{
		int_t x0 = javaIntAdd(x, javaIntNegate(javaIntMultiply(chunkX, 16)));
		int_t x1 = javaIntAdd(javaIntAdd(x, xSize), javaIntNegate(javaIntMultiply(chunkX, 16)));
		if (x0 < 0)
			x0 = 0;
		if (x1 > 16)
			x1 = 16;

		for (int_t chunkZ = chunkZ0; chunkZ <= chunkZ1; ++chunkZ)
		{
			int_t z0 = javaIntAdd(z, javaIntNegate(javaIntMultiply(chunkZ, 16)));
			int_t z1 = javaIntAdd(javaIntAdd(z, zSize), javaIntNegate(javaIntMultiply(chunkZ, 16)));
			if (z0 < 0)
				z0 = 0;
			if (z1 > 16)
				z1 = 16;

			int_t consumed = getChunk(chunkX, chunkZ)->setBlocksAndData(
				data.data() + offset, x0, y0, z0, x1, y1, z1);
			offset = javaIntAdd(offset, consumed);
			setTilesDirty(javaIntAdd(javaIntMultiply(chunkX, 16), x0), y0,
				javaIntAdd(javaIntMultiply(chunkZ, 16), z0),
				javaIntAdd(javaIntMultiply(chunkX, 16), x1), y1,
				javaIntAdd(javaIntMultiply(chunkZ, 16), z1));
		}
	}
}

void MultiplayerLevel::clearPendingBlockChanges(int_t x0, int_t y0, int_t z0,
	int_t x1, int_t y1, int_t z1)
{
	for (auto it = pendingBlockChanges.begin(); it != pendingBlockChanges.end();)
	{
		if (it->x >= x0 && it->y >= y0 && it->z >= z0 &&
			it->x <= x1 && it->y <= y1 && it->z <= z1)
		{
			it = pendingBlockChanges.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void MultiplayerLevel::disconnect()
{
	if (networkHandler != nullptr)
		networkHandler->sendAndQuit(std::make_unique<Packet255KickDisconnect>(u"Quitting"));
}

bool MultiplayerLevel::isRaining()
{
	return getRainStrength(1.0f) > 0.2f;
}

void MultiplayerLevel::setRaining(bool raining)
{
	this->raining = raining;
}

void MultiplayerLevel::setRainingStrength(float strength)
{
	setRainStrength(strength);
}

float MultiplayerLevel::getRainingStrength() const
{
	return rainingStrength;
}

float MultiplayerLevel::getPreviousRainingStrength() const
{
	return previousRainingStrength;
}
