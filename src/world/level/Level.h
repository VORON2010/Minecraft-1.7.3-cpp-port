#pragma once

#include "world/level/LevelSource.h"

#include <functional>
#include <memory>
#include <set>
#include <unordered_set>

#include "nbt/Tag.h"
#include "nbt/CompoundTag.h"

#include "java/Type.h"
#include "java/String.h"
#include "java/System.h"
#include "java/File.h"
#include "java/Random.h"

#include "world/entity/Entity.h"
#include "world/entity/player/Player.h"

#include "world/level/LightLayer.h"
#include "world/level/LevelListener.h"
#include "world/level/LightUpdate.h"

#include "world/level/MapDataBase.h"
#include "world/level/chunk/LevelChunk.h"
#include "world/level/chunk/ChunkSource.h"
#include "world/level/biome/BiomeSource.h"
#include "world/level/dimension/Dimension.h"
#include "world/level/tile/Tile.h"

#include "util/ProgressListener.h"

#include "util/Memory.h"

class Explosion;
class PathEntity;

class Level : public LevelSource
{
private:
	struct TickNextTickData
	{
		int_t x = 0;
		int_t y = 0;
		int_t z = 0;
		int_t tileId = 0;
		long_t delay = 0;
		long_t order = 0;

		bool operator==(const TickNextTickData &other) const
		{
			return x == other.x && y == other.y && z == other.z && tileId == other.tileId;
		}
	};

	struct TickNextTickOrder
	{
		bool operator()(const TickNextTickData &a, const TickNextTickData &b) const
		{
			if (a.delay != b.delay)
				return a.delay < b.delay;
			return a.order < b.order;
		}
	};

	struct TickNextTickHash
	{
		size_t operator()(const TickNextTickData &data) const
		{
			return static_cast<size_t>(((static_cast<uint_t>(data.x) * 128U * 1024U + static_cast<uint_t>(data.z) * 128U + static_cast<uint_t>(data.y)) * 256U) + static_cast<uint_t>(data.tileId));
		}
	};

	static constexpr int_t MAX_TICK_TILES_PER_TICK = 1000;
	static ulong_t nextTickEntryId;
	std::set<TickNextTickData, TickNextTickOrder> tickNextTickList;
	std::unordered_set<TickNextTickData, TickNextTickHash> tickNextTickSet;

public:
	static constexpr int_t MAX_LEVEL_SIZE = 32000000;
	static constexpr short_t DEPTH = 128;
	static constexpr short_t SEA_LEVEL = 64;
	
	bool instaTick = false;

	static constexpr int_t MAX_BRIGHTNESS = 15;
	static constexpr int_t TICKS_PER_DAY = 24000;

private:
public:
	std::vector<LightUpdate> lightUpdates;

public:
	std::vector<std::shared_ptr<Entity>> entities;
	std::vector<std::shared_ptr<Entity>> entitiesToRemove;
	std::vector<std::shared_ptr<Entity>> weatherEffects;

	std::vector<std::shared_ptr<TileEntity>> tileEntityList;

private:
	bool tickingTileEntities = false;
	std::vector<std::shared_ptr<TileEntity>> pendingTileEntities;

public:

	std::vector<std::shared_ptr<Player>> players;

	long_t time = 0;

private:
	long_t cloudColor = 0xFFFFFF;

public:
	int_t skyDarken = 0;
	int_t lastLightningBolt = 0;

protected:
	float previousRainingStrength = 0.0f;
	float rainingStrength = 0.0f;
	float previousThunderingStrength = 0.0f;
	float thunderingStrength = 0.0f;
	bool raining = false;
	int_t rainTime = 0;
	bool thundering = false;
	int_t thunderTime = 0;
	int_t randValue = Random().nextInt();
	int_t addend = 1013904223;

public:
	bool noNeighborUpdate = false;
	bool editingBlocks = false;

private:
	long_t sessionId = System::currentTimeMillis();

protected:
	int_t saveInterval = 40;

public:
	int_t difficulty = 0;
	
	Random random;

	int_t xSpawn = 0, ySpawn = 0, zSpawn = 0;

	bool isNew = false;

	std::shared_ptr<Dimension> dimension;

protected:
	std::vector<LevelListener *> listeners;

private:
	std::shared_ptr<ChunkSource> chunkSource;

public:
	std::shared_ptr<File> workDir, dir;

	long_t seed = 0;

private:
	std::shared_ptr<CompoundTag> loadedPlayerTag;

public:
	long_t sizeOnDisk = 0;
	jstring name;
	jstring levelName;
	bool isFindingSpawn = false;

private:
	std::vector<AABB *> boxes;
	int_t maxRecurse = 0;
	bool spawnEnemies = true;
	bool spawnFriendlies = true;

	std::shared_ptr<std::unordered_map<jstring, std::unique_ptr<MapDataBase>>> loadedItemData =
		std::make_shared<std::unordered_map<jstring, std::unique_ptr<MapDataBase>>>();
	std::shared_ptr<std::unordered_map<jstring, short_t>> idCounts =
		std::make_shared<std::unordered_map<jstring, short_t>>();

public:
	struct Summary
	{
		jstring folderName;
		jstring levelName;
		long_t lastPlayed = 0;
		long_t sizeOnDisk = 0;
		int_t version = 0;
		bool requiresConversion = true;
	};

	static int_t maxLoop;

private:
	int_t delayUntilNextMoodSound = random.nextInt(12000);
	std::vector<std::shared_ptr<Entity>> es;
	JavaTilePosSet activeChunks;
	void initializeStorage(File *workingDirectory);

public:
	bool isOnline = false;
	static std::shared_ptr<CompoundTag> getDataTagFor(File &workingDirectory, const jstring &name);
	static std::vector<Summary> getLevelList(File &workingDirectory);
	static bool renameLevel(File &workingDirectory, const jstring &folderName, const jstring &levelName);
	static void deleteLevel(File &workingDirectory, const jstring &name);
	static void deleteRecursive(std::vector<std::unique_ptr<File>> &files);

	BiomeSource &getBiomeSource() override;
	static long_t getLevelSize(File &workingDirectory, const jstring &name);
	static long_t calcSize(std::vector<std::unique_ptr<File>> &files);

	Level(File *workingDirectory, const jstring &name);
	Level(const jstring &name, int_t dimension, long_t seed);
	Level(Level &level, int_t dimension);
	Level(File *workingDirectory, const jstring &name, const jstring &levelName, long_t seed);
	Level(File *workingDirectory, const jstring &name, long_t seed);
	Level(File *workingDirectory, const jstring &name, const jstring &levelName, long_t seed, int_t dimension);
	Level(File *workingDirectory, const jstring &name, long_t seed, int_t dimension);
	static std::shared_ptr<Level> createSimulationLevel(File *workingDirectory, const jstring &name, long_t seed);

protected:
	Level(const jstring &name, int_t dimension, long_t seed, bool initializeChunkSource);
	virtual ChunkSource *createChunkSource(std::shared_ptr<File> dir);
	void setChunkSource(std::shared_ptr<ChunkSource> source);

public:
	void validateSpawn();
	int_t getTopTile(int_t x, int_t z);


	void clearLoadedPlayerData();
	virtual void setSpawnLocation();
	void updateEntityList();
	void loadPlayer(std::shared_ptr<Player> player);

	void save(bool force, std::shared_ptr<ProgressListener> progressRenderer);
	void saveLevelData();

	bool pauseSave(int_t saveStep);

	int_t getTile(int_t x, int_t y, int_t z) override;

	bool isEmptyTile(int_t x, int_t y, int_t z);

	bool hasChunkAt(int_t x, int_t y, int_t z);
	bool hasChunksAt(int_t x, int_t y, int_t z, int_t d);
	bool hasChunksAt(int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1);

private:
	bool hasChunk(int_t xc, int_t zc);

public:
	std::shared_ptr<LevelChunk> getChunkAt(int_t x, int_t z);
	std::shared_ptr<LevelChunk> getChunk(int_t xc, int_t zc);

	virtual bool setTileAndDataNoUpdate(int_t x, int_t y, int_t z, int_t tile, int_t data);
	virtual bool setTileNoUpdate(int_t x, int_t y, int_t z, int_t tile);

	const Material &getMaterial(int_t x, int_t y, int_t z) override;

	int_t getData(int_t x, int_t y, int_t z) override;
	void setData(int_t x, int_t y, int_t z, int_t data);
	virtual bool setDataNoUpdate(int_t x, int_t y, int_t z, int_t data);

	bool setTile(int_t x, int_t y, int_t z, int_t tile);
	bool setTileAndData(int_t x, int_t y, int_t z, int_t tile, int_t data);

	void sendTileUpdated(int_t x, int_t y, int_t z);
	void tileUpdated(int_t x, int_t y, int_t z, int_t tile);

	void lightColumnChanged(int_t x, int_t z, int_t y0, int_t y1);
	void setTileDirty(int_t x, int_t y, int_t z);
	void setTilesDirty(int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1);

	void swap(int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1);
	void updateNeighborsAt(int_t x, int_t y, int_t z, int_t tile);
	void neighborChanged(int_t x, int_t y, int_t z, int_t tile);
	void notifyBlockChange(int_t x, int_t y, int_t z, int_t tile);

	bool canSeeSky(int_t x, int_t y, int_t z);

	int_t getFullBrightness(int_t x, int_t y, int_t z);
	int_t getRawBrightness(int_t x, int_t y, int_t z);
	int_t getRawBrightness(int_t x, int_t y, int_t z, bool neighbors);

	bool isSkyLit(int_t x, int_t y, int_t z);
	int_t getHeightmap(int_t x, int_t z);
	int_t findTopSolidBlock(int_t x, int_t z);

	void updateLightIfOtherThan(int_t layer, int_t x, int_t y, int_t z, int_t brightness);

	int_t getBrightness(int_t layer, int_t x, int_t y, int_t z);
	void setBrightness(int_t layer, int_t x, int_t y, int_t z, int_t brightness);
	float getBrightness(int_t x, int_t y, int_t z) override;

	bool isDay();

	HitResult clip(Vec3 &from, Vec3 &to);
	HitResult clip(Vec3 &from, Vec3 &to, bool canPickLiquid);
	std::shared_ptr<Player> getNearestPlayer(Entity &entity, double radius);
	std::shared_ptr<Player> getNearestPlayer(double x, double y, double z, double radius);
	std::unique_ptr<PathEntity> getPathToEntity(Entity &entity, Entity &target, float distance);
	std::unique_ptr<PathEntity> getEntityPathToXYZ(Entity &entity, int_t x, int_t y, int_t z, float distance);
	std::shared_ptr<Entity> getEntityRef(Entity &entity);

	virtual bool addEntity(std::shared_ptr<Entity> entity);
	bool addWeatherEffect(std::shared_ptr<Entity> entity);
	const std::vector<std::shared_ptr<Entity>> &getWeatherEffects() const;

protected:
	virtual void entityAdded(std::shared_ptr<Entity> entity);
	virtual void entityRemoved(std::shared_ptr<Entity> entity);

public:
	virtual void removeEntity(std::shared_ptr<Entity> entity);
	virtual void removeEntityImmediately(std::shared_ptr<Entity> entity);

	void addListener(LevelListener &listener);
	void removeListener(LevelListener &listener);

	
	void playSoundAtEntity(Entity &entity, const jstring &name, float volume, float pitch);
	void playSoundEffect(double x, double y, double z, const jstring &name, float volume, float pitch);
	void playRecord(const jstring &name, int_t x, int_t y, int_t z);
	void addParticle(const jstring &name, double x, double y, double z, double xa, double ya, double za);
	void levelEvent(int_t event, int_t x, int_t y, int_t z, int_t data);
	void levelEvent(Player *player, int_t event, int_t x, int_t y, int_t z, int_t data);

	const std::vector<AABB *> &getCubes(Entity &entity, AABB &bb);

	int_t getSkyDarken(float a);
	Vec3 *getSkyColor(Entity &entity, float a);
	float getTimeOfDay(float a);
	float getSunAngle(float a);
	Vec3 *getCloudColor(float a);
	Vec3 *getFogColor(float a);

	int_t getTopSolidBlock(int_t x, int_t z);
	int_t getLightDepth(int_t x, int_t z);

	float getStarBrightness(float a);

	void addToTickNextTick(int_t x, int_t y, int_t z, int_t tileId);

	void tickEntities();
	void tick(std::shared_ptr<Entity> entity);
	void tick(std::shared_ptr<Entity> entity, bool ai);

	bool isUnobstructed(AABB &bb);
	bool containsAnyLiquid(AABB &bb);
	bool isMaterialInBB(AABB &bb, const Material &material);
	bool handleMaterialAcceleration(AABB &bb, const Material &material, Entity &entity);
	bool containsFireTile(AABB &bb);

	void extinguishFire(int_t x, int_t y, int_t z, Facing f);

	jstring gatherStats();
	jstring gatherChunkSourceStats();

	std::shared_ptr<TileEntity> getTileEntity(int_t x, int_t y, int_t z) override;
	void setTileEntity(int_t x, int_t y, int_t z, std::shared_ptr<TileEntity> tileEntity);
	void removeTileEntity(int_t x, int_t y, int_t z);
	void addTileEntities(const std::vector<std::shared_ptr<TileEntity>> &tileEntities);

	bool isSolidTile(int_t x, int_t y, int_t z) override;
	void forceSave(std::shared_ptr<ProgressListener> progressRenderer);

	int_t getLightsToUpdate();

	bool updateLights();

	void updateLight(int_t layer, int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1);
	void updateLight(int_t layer, int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1, bool checkExpansion);
	void updateSkyBrightness();

	void setSimulationSeed(long_t seed);
	void setSpawnSettings(bool spawnEnemies, bool spawnFriendlies);

	virtual void tick();

protected:
	void tickTiles();

public:
	virtual bool tickPendingTicks(bool unknown);
	void animateTick(int_t x, int_t y, int_t z);

	const std::vector<std::shared_ptr<Entity>> &getEntities(Entity *ignore, AABB &aabb);
	std::vector<std::shared_ptr<Entity>> getEntitiesOfCondition(bool (*condition)(Entity&), AABB &aabb);
	const std::vector<std::shared_ptr<Entity>> &getAllEntities();

	void tileEntityChanged(int_t x, int_t y, int_t z, std::shared_ptr<TileEntity> tileEntity);

	int_t countConditionOf(bool (*condition)(Entity&));
	void addEntities(const std::vector<std::shared_ptr<Entity>> &entities);
	void removeEntities(const std::vector<std::shared_ptr<Entity>> &entities);

	virtual void disconnect();

	void checkSession();

	void setTime(long_t time);

	void ensureAdded(std::shared_ptr<Entity> entity);

	bool mayInteract(std::shared_ptr<Player> player, int_t x, int_t y, int_t z);

	void broadcastEntityEvent(std::shared_ptr<Entity> entity, byte_t event);

	bool isBlockNormalCube(int_t x, int_t y, int_t z);
	bool getDirectSignal(int_t x, int_t y, int_t z, int_t dir);
	bool hasDirectSignal(int_t x, int_t y, int_t z);
	bool getSignal(int_t x, int_t y, int_t z, int_t dir);
	bool hasNeighborSignal(int_t x, int_t y, int_t z);
	void notifyBlocksOfNeighborChange(int_t x, int_t y, int_t z, int_t tileId);
	virtual void scheduleBlockUpdate(int_t x, int_t y, int_t z, int_t tileId, int_t delay);
	Explosion createExplosion(Entity *entity, double x, double y, double z, float size);
	Explosion createExplosion(Entity *entity, double x, double y, double z, float size, bool flaming);
	void playNoteAt(int_t x, int_t y, int_t z, int_t type, int_t data);
	std::shared_ptr<ChunkSource> getChunkSource();

	virtual bool isRaining();
	virtual bool isThundering();
	virtual float getRainStrength(float a) const;
	virtual float getThunderStrength(float a) const;
	void setWeather(bool rain, bool thunder);

protected:
	virtual void updateWeather();
	void stopPrecipitation();

public:
	virtual void setRainStrength(float strength);
	bool canBlockBeRainedOn(int_t x, int_t y, int_t z);

	MapDataBase *loadItemData(const jstring &id, const std::function<std::unique_ptr<MapDataBase>(const jstring&)> &factory);
	void setItemData(const jstring &id, MapDataBase *data);
	void shareItemDataWith(Level &level);
	int_t getUniqueDataId(const jstring &key);
	void saveAllItemData();

	bool allPlayersSleeping = false;
	void updateAllPlayersSleepingFlag();
	void wakeUpAllPlayers();
	bool isAllPlayersFullyAsleep();
	bool performSleepSpawning();
};
