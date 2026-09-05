#pragma once

#include <list>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "world/level/Level.h"

class MultiplayerChunkCache;
class NetClientHandler;

class MultiplayerLevel : public Level
{
private:
	struct PendingBlockChange
	{
		int_t x;
		int_t y;
		int_t z;
		int_t ticks = 80;
		int_t tile;
		int_t data;

		PendingBlockChange(int_t x, int_t y, int_t z, int_t tile, int_t data)
			: x(x), y(y), z(z), tile(tile), data(data) {}
	};

	std::list<PendingBlockChange> pendingBlockChanges;
	NetClientHandler *networkHandler;
	std::shared_ptr<MultiplayerChunkCache> multiplayerChunkCache;
	std::unordered_map<int_t, std::shared_ptr<Entity>> entitiesById;
	std::unordered_set<std::shared_ptr<Entity>> trackedEntities;
	std::unordered_set<std::shared_ptr<Entity>> pendingEntities;

	bool raining = false;

	void addPendingBlockChange(int_t x, int_t y, int_t z, int_t tile, int_t data);

protected:
	void entityAdded(std::shared_ptr<Entity> entity) override;
	void entityRemoved(std::shared_ptr<Entity> entity) override;

public:
	MultiplayerLevel(NetClientHandler &networkHandler, long_t seed, int_t dimension);

	void setSpawnLocation() override;
	void tick() override;
	void scheduleBlockUpdate(int_t x, int_t y, int_t z, int_t tileId, int_t delay) override;
	bool tickPendingTicks(bool unknown) override;

	void doPreChunk(int_t x, int_t z, bool load);
	MultiplayerChunkCache &getMultiplayerChunkCache();

	bool addEntity(std::shared_ptr<Entity> entity) override;
	void removeEntity(std::shared_ptr<Entity> entity) override;
	void addEntityById(int_t entityId, std::shared_ptr<Entity> entity);
	std::shared_ptr<Entity> getEntityById(int_t entityId) const;
	std::shared_ptr<Entity> removeEntityById(int_t entityId);

	bool setDataNoUpdate(int_t x, int_t y, int_t z, int_t data) override;
	bool setTileAndDataNoUpdate(int_t x, int_t y, int_t z, int_t tile, int_t data) override;
	bool setTileNoUpdate(int_t x, int_t y, int_t z, int_t tile) override;
	bool setTileAndDataRemote(int_t x, int_t y, int_t z, int_t tile, int_t data);
	void setChunkData(int_t x, int_t y, int_t z, int_t xSize, int_t ySize, int_t zSize,
		const std::vector<byte_t> &data);
	void clearPendingBlockChanges(int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1);

	void disconnect() override;
	bool isRaining() override;
	void setRaining(bool raining);
	void setRainingStrength(float strength);
	float getRainingStrength() const;
	float getPreviousRainingStrength() const;
};
