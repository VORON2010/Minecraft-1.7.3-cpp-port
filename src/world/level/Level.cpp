#include "world/level/Level.h"
#include "world/level/SpawnerAnimals.h"
#include "MinecraftException.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <set>
#include <stdexcept>

#include "nbt/NbtIo.h"

#include "world/level/MapDataBase.h"
#include "world/level/chunk/ChunkCache.h"
#include "world/level/Region.h"
#include "world/level/pathfinder/Pathfinder.h"
#include "world/entity/animal/Animal.h"
#include "world/entity/animal/Chicken.h"
#include "world/entity/animal/Cow.h"
#include "world/entity/animal/Pig.h"
#include "world/entity/animal/Sheep.h"
#include "world/entity/animal/Squid.h"
#include "world/entity/animal/Wolf.h"
#include "world/entity/monster/Creeper.h"
#include "world/entity/monster/Monster.h"
#include "world/entity/monster/Skeleton.h"
#include "world/entity/monster/Spider.h"
#include "world/entity/monster/Zombie.h"
#include "world/entity/monster/Slime.h"
#include "world/entity/monster/Ghast.h"
#include "world/entity/monster/PigZombie.h"
#include "world/level/biome/BiomeSource.h"
#include "world/level/material/Material.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/level/pathfinder/PathEntity.h"
#include "world/level/material/GasMaterial.h"

#include "java/File.h"
#include "java/IOUtil.h"

#include "world/level/tile/LiquidTile.h"
#include "world/level/tile/FireTile.h"
#include "world/level/tile/BedTile.h"
#include "world/level/tile/SnowTile.h"
#include "world/level/tile/IceTile.h"
#include "world/entity/EntityLightningBolt.h"
#include "world/level/Explosion.h"
#include "util/Mth.h"
#include "util/Profiler.h"

int_t Level::maxLoop = 0;
ulong_t Level::nextTickEntryId = 0;

struct LevelLightRecursion
{
	int_t &depth;
	explicit LevelLightRecursion(int_t &depth) : depth(depth) { ++depth; }
	~LevelLightRecursion() { --depth; }
};

namespace
{

jstring getFileName(const File &file)
{
	jstring path = file.toString();
	size_t pos = path.find_last_of(u"/\\");
	return pos == jstring::npos ? path : path.substr(pos + 1);
}

std::shared_ptr<CompoundTag> readLevelDataFile(File &worldDir, const jstring &fileName)
{
	std::unique_ptr<File> levelDat(File::open(worldDir, fileName));
	if (!levelDat->exists())
		return nullptr;

	std::unique_ptr<std::istream> is(levelDat->toStreamIn());
	if (!is)
		return nullptr;

	try
	{
		std::unique_ptr<CompoundTag> tag(NbtIo::readCompressed(*is));
		return tag->getCompound(u"Data");
	}
	catch (const std::exception &)
	{
		return nullptr;
	}
}


std::shared_ptr<CompoundTag> readLevelData(File &worldDir)
{
	std::shared_ptr<CompoundTag> data = readLevelDataFile(worldDir, u"level.dat");
	if (data == nullptr)
		data = readLevelDataFile(worldDir, u"level.dat_old");
	return data;
}

void writeLevelData(File &worldDir, const std::shared_ptr<CompoundTag> &data)
{
	std::shared_ptr<CompoundTag> root = Util::make_shared<CompoundTag>();
	root->put(u"Data", data);

	std::unique_ptr<File> fileNewDat(File::open(worldDir, u"level.dat_new"));
	std::unique_ptr<File> fileOldDat(File::open(worldDir, u"level.dat_old"));
	std::unique_ptr<File> fileDat(File::open(worldDir, u"level.dat"));

	std::unique_ptr<std::ostream> os(fileNewDat->toStreamOut());
	if (!os)
		throw std::runtime_error("Failed to open level.dat_new for writing");
	NbtIo::writeCompressed(*root, *os);
	os->flush();
	os.reset();

	if (fileOldDat->exists())
		fileOldDat->remove();
	fileDat->renameTo(*fileOldDat);
	if (fileDat->exists())
		fileDat->remove();
	fileNewDat->renameTo(*fileDat);
	if (fileNewDat->exists())
		fileNewDat->remove();
}

}

std::shared_ptr<CompoundTag> Level::getDataTagFor(File &workingDirectory, const jstring &name)
{
	std::unique_ptr<File> saves(File::open(workingDirectory, u"saves"));
	std::unique_ptr<File> world(File::open(*saves, name));
	if (!world->exists())
		return nullptr;
	return readLevelData(*world);
}

std::vector<Level::Summary> Level::getLevelList(File &workingDirectory)
{
	std::vector<Summary> levels;
	std::unique_ptr<File> saves(File::open(workingDirectory, u"saves"));
	if (!saves->exists())
		return levels;

	for (auto &world : saves->listFiles())
	{
		if (!world->isDirectory())
			continue;

		std::shared_ptr<CompoundTag> data = readLevelData(*world);
		if (data == nullptr)
			continue;

		Summary summary;
		summary.folderName = getFileName(*world);
		summary.levelName = data->contains(u"LevelName") ? data->getString(u"LevelName") : summary.folderName;
		if (summary.levelName.empty())
			summary.levelName = summary.folderName;
		summary.lastPlayed = data->getLong(u"LastPlayed");
		summary.sizeOnDisk = data->contains(u"SizeOnDisk") ? data->getLong(u"SizeOnDisk") : 0;
		if (summary.sizeOnDisk <= 0)
		{
			auto worldFiles = world->listFiles();
			summary.sizeOnDisk = calcSize(worldFiles);
		}
		summary.version = data->contains(u"version") ? data->getInt(u"version") : 0;
		summary.requiresConversion = summary.version != 19132;
		levels.push_back(summary);
	}

	std::sort(levels.begin(), levels.end(), [](const Summary &a, const Summary &b) {
		if (a.lastPlayed != b.lastPlayed)
			return a.lastPlayed > b.lastPlayed;
		return a.folderName < b.folderName;
	});

	return levels;
}

bool Level::renameLevel(File &workingDirectory, const jstring &folderName, const jstring &levelName)
{
	std::unique_ptr<File> saves(File::open(workingDirectory, u"saves"));
	std::unique_ptr<File> world(File::open(*saves, folderName));
	if (!world->exists())
		return false;

	std::shared_ptr<CompoundTag> data = readLevelData(*world);
	if (data == nullptr)
		return false;

	data->putString(u"LevelName", levelName);
	writeLevelData(*world, data);
	return true;
}

void Level::deleteLevel(File &workingDirectory, const jstring &name)
{
	std::unique_ptr<File> saves(File::open(workingDirectory, u"saves"));
	std::unique_ptr<File> world(File::open(*saves, name));
	if (!world->exists())
		return;

	auto level_files = world->listFiles();
	deleteRecursive(level_files);
	world->remove();
}

void Level::deleteRecursive(std::vector<std::unique_ptr<File>> &files)
{
	for (auto &i : files)
	{
		if (i->isDirectory())
		{
			auto i_files = i->listFiles();
			deleteRecursive(i_files);
		}
		i->remove();
	}
}

BiomeSource &Level::getBiomeSource()
{
	return *dimension->biomeSource;
}

long_t Level::getLevelSize(File &workingDirectory, const jstring &name)
{
	std::unique_ptr<File> saves(File::open(workingDirectory, u"saves"));
	std::unique_ptr<File> world(File::open(*saves, name));
	if (!world->exists())
		return 0;
	auto level_files = world->listFiles();
	return calcSize(level_files);
}

long_t Level::calcSize(std::vector<std::unique_ptr<File>> &files)
{
	long_t sizeOnDisk = 0;
	for (auto &i : files)
	{
		if (i->isDirectory())
		{
			auto i_files = i->listFiles();
			sizeOnDisk += calcSize(i_files);
		}
		else
		{
			sizeOnDisk += i->length();
		}
	}
	return sizeOnDisk;
}

Level::Level(File *workingDirectory, const jstring &name) : Level(workingDirectory, name, name, Random().nextLong())
{

}

Level::Level(const jstring &name, int_t dimension, long_t seed) : Level(name, dimension, seed, true)
{
}

Level::Level(const jstring &name, int_t dimension, long_t seed, bool initializeChunkSource)
{
	this->name = name;
	this->levelName = name;
	this->seed = seed;
	this->dimension.reset(Dimension::getNew(*this, dimension));
	if (initializeChunkSource)
		this->chunkSource.reset(createChunkSource(dir));
	updateSkyBrightness();
}

Level::Level(Level &level, int_t dimension)
{
	this->sessionId = level.sessionId;
	this->workDir = level.workDir;
	this->dir = level.dir;
	this->name = level.name;
	this->levelName = level.levelName;
	this->seed = level.seed;
	this->time = level.time;
	this->xSpawn = level.xSpawn;
	this->ySpawn = level.ySpawn;
	this->zSpawn = level.zSpawn;
	this->sizeOnDisk = level.sizeOnDisk;
	this->dimension.reset(Dimension::getNew(*this, dimension));
	this->chunkSource.reset(createChunkSource(dir));
	updateSkyBrightness();
}

Level::Level(File *workingDirectory, const jstring &name, const jstring &levelName, long_t seed) : Level(workingDirectory, name, levelName, seed, Dimension::Id_None)
{

}

Level::Level(File *workingDirectory, const jstring &name, long_t seed) : Level(workingDirectory, name, name, seed, Dimension::Id_None)
{

}

void Level::initializeStorage(File *workingDirectory)
{
	workDir.reset(workingDirectory);
	workingDirectory->mkdirs();
	dir.reset(File::open(*workingDirectory, name));
	dir->mkdirs();
	std::unique_ptr<File> sessionLock(File::open(*dir, u"session.lock"));
	std::unique_ptr<std::ostream> output(sessionLock->toStreamOut());
	if (!output)
		throw std::runtime_error("Failed to check session lock, aborting");
	IOUtil::writeLong(*output, sessionId);
}

std::shared_ptr<Level> Level::createSimulationLevel(File *workingDirectory, const jstring &name, long_t seed)
{
	auto level = std::shared_ptr<Level>(new Level(name, Dimension::Id_Normal, seed, false));
	level->initializeStorage(workingDirectory);
	level->setSimulationSeed(seed);
	level->chunkSource.reset(level->createChunkSource(level->dir));
	level->xSpawn = 0;
	level->ySpawn = 96;
	level->zSpawn = 0;
	return level;
}

Level::Level(File *workingDirectory, const jstring &name, const jstring &levelName, long_t seed, int_t dimension)
{
	this->name = name;
	this->levelName = levelName;
	initializeStorage(workingDirectory);

	int_t new_dimension = Dimension::Id_Normal;
	
	
	std::shared_ptr<CompoundTag> data = readLevelData(*dir);
	isNew = data == nullptr;

	if (data != nullptr)
	{
		this->seed = data->getLong(u"RandomSeed");
		xSpawn = data->getInt(u"SpawnX");
		ySpawn = data->getInt(u"SpawnY");
		zSpawn = data->getInt(u"SpawnZ");
		time = data->getLong(u"Time");
		sizeOnDisk = data->getLong(u"SizeOnDisk");
		this->levelName = data->contains(u"LevelName") ? data->getString(u"LevelName") : name;
		if (this->levelName.empty())
			this->levelName = name;

		rainTime = data->getInt(u"rainTime");
		raining = data->getBoolean(u"raining");
		thunderTime = data->getInt(u"thunderTime");
		thundering = data->getBoolean(u"thundering");
		
		if (raining)
		{
			rainingStrength = 1.0f;
			if (thundering)
				thunderingStrength = 1.0f;
		}

		if (data->contains(u"Player"))
		{
			loadedPlayerTag = data->getCompound(u"Player");

			int_t tag_dimension = loadedPlayerTag->getInt(u"Dimension");
			if (tag_dimension == Dimension::Id_Hell)
				dimension = tag_dimension;
		}
	}

	if (dimension != Dimension::Id_None)
		new_dimension = dimension;

	bool findSpawn = false;
	if (this->seed == 0)
	{
		this->seed = seed;
		findSpawn = true;
	}

	if (this->levelName.empty())
		this->levelName = name;

	this->dimension.reset(Dimension::getNew(*this, new_dimension));
	this->chunkSource.reset(createChunkSource(dir));

	if (findSpawn)
	{
		isFindingSpawn = true;
		xSpawn = 0;
		ySpawn = 64;
		zSpawn = 0;
		while (!this->dimension->isValidSpawn(xSpawn, zSpawn))
		{
			xSpawn += random.nextInt(64) - random.nextInt(64);
			zSpawn += random.nextInt(64) - random.nextInt(64);
		}
		isFindingSpawn = false;
	}
}

Level::Level(File *workingDirectory, const jstring &name, long_t seed, int_t dimension) : Level(workingDirectory, name, name, seed, dimension)
{

}

ChunkSource *Level::createChunkSource(std::shared_ptr<File> dir)
{
	return new ChunkCache(*this, dimension->createStorage(dir), dimension->createRandomLevelSource());
}

void Level::setChunkSource(std::shared_ptr<ChunkSource> source)
{
	chunkSource = std::move(source);
}

void Level::validateSpawn()
{

}

int_t Level::getTopTile(int_t x, int_t z)
{
	int_t y = 63;
	while (!isEmptyTile(x, y + 1, z))
		++y;
	return getTile(x, y, z);
}

void Level::clearLoadedPlayerData()
{
}

void Level::setSpawnLocation()
{
	if (ySpawn <= 0)
		ySpawn = 64;

	int_t x = xSpawn;
	int_t z = zSpawn;
	while (getTopTile(x, z) == 0)
	{
		x += random.nextInt(8) - random.nextInt(8);
		z += random.nextInt(8) - random.nextInt(8);
	}

	xSpawn = x;
	zSpawn = z;
}

void Level::updateEntityList()
{
	entities.erase(std::remove_if(entities.begin(), entities.end(), [&](const auto &entity) {
		return std::any_of(entitiesToRemove.begin(), entitiesToRemove.end(), [&](const auto &removed) {
			return entity->entityId == removed->entityId;
		});
	}), entities.end());

	for (size_t i = 0; i < entitiesToRemove.size(); ++i)
	{
		auto entity = entitiesToRemove[i];
		if (entity->inChunk && hasChunk(entity->xChunk, entity->zChunk))
			getChunk(entity->xChunk, entity->zChunk)->removeEntity(entity);
	}
	for (size_t i = 0; i < entitiesToRemove.size(); ++i)
		entityRemoved(entitiesToRemove[i]);
	entitiesToRemove.clear();

	for (size_t i = 0; i < entities.size();)
	{
		auto entity = entities[i];
		if (entity->riding != nullptr)
		{
			if (!entity->riding->removed && entity->riding->rider == entity)
			{
				++i;
				continue;
			}
			entity->riding->rider = nullptr;
			entity->riding = nullptr;
		}
		if (!entity->removed)
		{
			++i;
			continue;
		}
		if (entity->inChunk && hasChunk(entity->xChunk, entity->zChunk))
			getChunk(entity->xChunk, entity->zChunk)->removeEntity(entity);
		if (i >= entities.size())
			throw std::out_of_range("Entity list index removed during update");
		entities.erase(entities.begin() + i);
		entityRemoved(entity);
	}
}



void Level::loadPlayer(std::shared_ptr<Player> player)
{
	if (loadedPlayerTag != nullptr)
	{
		player->load(*loadedPlayerTag);
		loadedPlayerTag = nullptr;
	}


	addEntity(player);
}

void Level::save(bool force, std::shared_ptr<ProgressListener> progressRenderer)
{
	if (!chunkSource->shouldSave())
		return;

	if (progressRenderer != nullptr)
		progressRenderer->progressStartNoAbort(u"Saving level");
	saveLevelData();
	saveAllItemData();
	if (progressRenderer != nullptr)
		progressRenderer->progressStage(u"Saving chunks");
	chunkSource->save(force, progressRenderer);
}

void Level::saveLevelData()
{
	checkSession();

	std::shared_ptr<CompoundTag> data = Util::make_shared<CompoundTag>();
	data->putLong(u"RandomSeed", seed);
	data->putInt(u"SpawnX", xSpawn);
	data->putInt(u"SpawnY", ySpawn);
	data->putInt(u"SpawnZ", zSpawn);
	data->putLong(u"Time", time);
	data->putLong(u"SizeOnDisk", sizeOnDisk);
	data->putLong(u"LastPlayed", System::currentTimeMillis());
	data->putString(u"LevelName", levelName);
	data->putInt(u"version", 19132);
	data->putInt(u"rainTime", rainTime);
	data->putBoolean(u"raining", raining);
	data->putInt(u"thunderTime", thunderTime);
	data->putBoolean(u"thundering", thundering);

	std::shared_ptr<Player> player;
	if (!players.empty())
		player = players[0];

	if (player != nullptr)
	{
		std::shared_ptr<CompoundTag> playerTag = Util::make_shared<CompoundTag>();
		player->saveWithoutId(*playerTag);
		data->put(u"Player", playerTag);
	}

	writeLevelData(*dir, data);
}

bool Level::pauseSave(int_t saveStep)
{
	if (!chunkSource->shouldSave())
		return true;
	if (saveStep == 0)
		saveLevelData();
	return chunkSource->save(false, nullptr);
}

int_t Level::getTile(int_t x, int_t y, int_t z)
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z > MAX_LEVEL_SIZE)
		return 0;
	if (y < 0)
		return 0;
	if (y >= DEPTH)
		return 0;
	std::shared_ptr<LevelChunk> chunk = getChunk(x >> 4, z >> 4);
	return chunk->getTile(x & 0xF, y, z & 0xF);
}

bool Level::isEmptyTile(int_t x, int_t y, int_t z)
{
	return getTile(x, y, z) == 0;
}

bool Level::hasChunkAt(int_t x, int_t y, int_t z)
{
	return y >= 0 && y < DEPTH && hasChunk(x >> 4, z >> 4);
}

bool Level::hasChunksAt(int_t x, int_t y, int_t z, int_t d)
{
	return hasChunksAt(x - d, y - d, z - d, x + d, y + d, z + d);
}

bool Level::hasChunksAt(int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1)
{
	if (y1 < 0 || y0 >= DEPTH)
		return false;

	x0 >>= 4;
	y0 >>= 4;
	z0 >>= 4;
	x1 >>= 4;
	y1 >>= 4;
	z1 >>= 4;

	for (int_t cx = x0; cx <= x1; cx++)
		for (int_t cz = z0; cz <= z1; cz++)
			if (!hasChunk(cx, cz))
				return false;
	return true;
}

bool Level::hasChunk(int_t xc, int_t zc)
{
	return chunkSource->hasChunk(xc, zc);
}

std::shared_ptr<LevelChunk> Level::getChunkAt(int_t x, int_t z)
{
	return getChunk(x >> 4, z >> 4);
}

std::shared_ptr<LevelChunk> Level::getChunk(int_t xc, int_t zc)
{
	return chunkSource->getChunk(xc, zc);
}

bool Level::setTileAndDataNoUpdate(int_t x, int_t y, int_t z, int_t tile, int_t data)
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z > MAX_LEVEL_SIZE)
		return false;
	if (y < 0)
		return false;
	if (y >= DEPTH)
		return false;
	std::shared_ptr<LevelChunk> chunk = getChunk(x >> 4, z >> 4);
	return chunk->setTileAndData(x & 0xF, y, z & 0xF, tile, data);
}

bool Level::setTileNoUpdate(int_t x, int_t y, int_t z, int_t tile)
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z > MAX_LEVEL_SIZE)
		return false;
	if (y < 0)
		return false;
	if (y >= DEPTH)
		return false;
	std::shared_ptr<LevelChunk> chunk = getChunk(x >> 4, z >> 4);
	return chunk->setTile(x & 0xF, y, z & 0xF, tile);
}

const Material &Level::getMaterial(int_t x, int_t y, int_t z)
{
	int_t tile = getTile(x, y, z);
	return (tile == 0) ? Material::air : Tile::tiles[tile]->material;
}

int_t Level::getData(int_t x, int_t y, int_t z)
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z > MAX_LEVEL_SIZE)
		return 0;
	if (y < 0)
		return 0;
	if (y >= DEPTH)
		return 0;
	std::shared_ptr<LevelChunk> chunk = getChunk(x >> 4, z >> 4);
	return chunk->getData(x & 0xF, y, z & 0xF);
}

void Level::setData(int_t x, int_t y, int_t z, int_t data)
{
	if (setDataNoUpdate(x, y, z, data))
	{
		int_t tile = getTile(x, y, z);
		if (Tile::notifyRenderOnDataChange[tile & 255])
			notifyBlockChange(x, y, z, tile);
		else
			notifyBlocksOfNeighborChange(x, y, z, tile);
	}
}

bool Level::setDataNoUpdate(int_t x, int_t y, int_t z, int_t data)
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z > MAX_LEVEL_SIZE)
		return false;
	if (y < 0)
		return false;
	if (y >= DEPTH)
		return false;
	std::shared_ptr<LevelChunk> chunk = getChunk(x >> 4, z >> 4);
	chunk->setData(x & 0xF, y, z & 0xF, data);
	return true;
}

bool Level::setTile(int_t x, int_t y, int_t z, int_t tile)
{
	if (setTileNoUpdate(x, y, z, tile))
	{
		notifyBlockChange(x, y, z, tile);
		return true;
	}
	return false;
}

bool Level::setTileAndData(int_t x, int_t y, int_t z, int_t tile, int_t data)
{
	if (setTileAndDataNoUpdate(x, y, z, tile, data))
	{
		notifyBlockChange(x, y, z, tile);
		return true;
	}
	return false;
}

void Level::sendTileUpdated(int_t x, int_t y, int_t z)
{
	for (size_t i = 0; i < listeners.size(); ++i)
		listeners[i]->tileChanged(x, y, z);
}

void Level::tileUpdated(int_t x, int_t y, int_t z, int_t tile)
{
	
	notifyBlockChange(x, y, z, tile);
}

void Level::lightColumnChanged(int_t x, int_t z, int_t y0, int_t y1)
{
	if (y0 > y1)
	{
		int_t temp = y0;
		y0 = y1;
		y1 = temp;
	}
	setTilesDirty(x, y0, z, x, y1, z);
}

void Level::setTileDirty(int_t x, int_t y, int_t z)
{
	for (size_t i = 0; i < listeners.size(); ++i)
		listeners[i]->setTilesDirty(x, y, z, x, y, z);
}

void Level::setTilesDirty(int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1)
{
	for (size_t i = 0; i < listeners.size(); ++i)
		listeners[i]->setTilesDirty(x0, y0, z0, x1, y1, z1);
}

void Level::swap(int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1)
{
	int_t t0 = getTile(x0, y0, z0);
	int_t d0 = getData(x0, y0, z0);
	int_t t1 = getTile(x1, y1, z1);
	int_t d1 = getData(x1, y1, z1);

	setTileAndDataNoUpdate(x0, y0, z0, t1, d1);
	setTileAndDataNoUpdate(x1, y1, z1, t0, d0);
	updateNeighborsAt(x0, y0, z0, t1);
	updateNeighborsAt(x1, y1, z1, t0);
}

void Level::updateNeighborsAt(int_t x, int_t y, int_t z, int_t tile)
{
	neighborChanged(x - 1, y, z, tile);
	neighborChanged(x + 1, y, z, tile);
	neighborChanged(x, y - 1, z, tile);
	neighborChanged(x, y + 1, z, tile);
	neighborChanged(x, y, z - 1, tile);
	neighborChanged(x, y, z + 1, tile);
}

void Level::neighborChanged(int_t x, int_t y, int_t z, int_t tile)
{
	if (noNeighborUpdate || isOnline)
		return;
	Tile *ptile = Tile::tiles[getTile(x, y, z)];
	if (ptile != nullptr)
		ptile->neighborChanged(*this, x, y, z, tile);
}

void Level::notifyBlockChange(int_t x, int_t y, int_t z, int_t tile)
{
	sendTileUpdated(x, y, z);
	notifyBlocksOfNeighborChange(x, y, z, tile);
}

bool Level::isBlockNormalCube(int_t x, int_t y, int_t z)
{
	int_t id = getTile(x, y, z);
	if (id == 0)
		return false;
	Tile *t = Tile::tiles[id];
	if (t == nullptr)
		return false;
	return t->material.isSolidBlocking() && t->isCubeShaped();
}



bool Level::getDirectSignal(int_t x, int_t y, int_t z, int_t dir)
{
	int_t id = getTile(x, y, z);
	if (id == 0)
		return false;
	Tile *t = Tile::tiles[id];
	if (t == nullptr)
		return false;
	return t->getDirectSignal(*this, x, y, z, dir);
}

bool Level::hasDirectSignal(int_t x, int_t y, int_t z)
{
	return getDirectSignal(x, y - 1, z, 0)
		|| getDirectSignal(x, y + 1, z, 1)
		|| getDirectSignal(x, y, z - 1, 2)
		|| getDirectSignal(x, y, z + 1, 3)
		|| getDirectSignal(x - 1, y, z, 4)
		|| getDirectSignal(x + 1, y, z, 5);
}



bool Level::getSignal(int_t x, int_t y, int_t z, int_t dir)
{
	if (isBlockNormalCube(x, y, z))
		return hasDirectSignal(x, y, z);
	int_t id = getTile(x, y, z);
	if (id == 0)
		return false;
	Tile *t = Tile::tiles[id];
	if (t == nullptr)
		return false;
	return t->getSignal(*this, x, y, z, dir);
}

bool Level::hasNeighborSignal(int_t x, int_t y, int_t z)
{
	return getSignal(x, y - 1, z, 0)
		|| getSignal(x, y + 1, z, 1)
		|| getSignal(x, y, z - 1, 2)
		|| getSignal(x, y, z + 1, 3)
		|| getSignal(x - 1, y, z, 4)
		|| getSignal(x + 1, y, z, 5);
}

void Level::notifyBlocksOfNeighborChange(int_t x, int_t y, int_t z, int_t tileId)
{
	neighborChanged(x - 1, y, z, tileId);
	neighborChanged(x + 1, y, z, tileId);
	neighborChanged(x, y - 1, z, tileId);
	neighborChanged(x, y + 1, z, tileId);
	neighborChanged(x, y, z - 1, tileId);
	neighborChanged(x, y, z + 1, tileId);
}

void Level::scheduleBlockUpdate(int_t x, int_t y, int_t z, int_t tileId, int_t delay)
{
	TickNextTickData entry;
	entry.x = x;
	entry.y = y;
	entry.z = z;
	entry.tileId = tileId;
	entry.order = nextTickEntryId++;

	constexpr int_t chunkRadius = 8;
	if (instaTick)
	{
		if (hasChunksAt(entry.x - chunkRadius, entry.y - chunkRadius, entry.z - chunkRadius, entry.x + chunkRadius, entry.y + chunkRadius, entry.z + chunkRadius))
		{
			int_t tile = getTile(entry.x, entry.y, entry.z);
			if (tile == entry.tileId && tile > 0 && Tile::tiles[tile] != nullptr)
				Tile::tiles[tile]->tick(*this, entry.x, entry.y, entry.z, random);
		}
		return;
	}

	if (!hasChunksAt(x - chunkRadius, y - chunkRadius, z - chunkRadius, x + chunkRadius, y + chunkRadius, z + chunkRadius))
		return;

	entry.delay = static_cast<long_t>(delay) + time;

	if (tickNextTickSet.find(entry) != tickNextTickSet.end())
		return;

	tickNextTickSet.insert(entry);
	tickNextTickList.insert(entry);
}

Explosion Level::createExplosion(Entity *entity, double x, double y, double z, float size)
{
	return createExplosion(entity, x, y, z, size, false);
}

Explosion Level::createExplosion(Entity *entity, double x, double y, double z, float size, bool flaming)
{
	Explosion explosion(*this, entity, x, y, z, size);
	explosion.isFlaming = flaming;
	explosion.doExplosionA();
	explosion.doExplosionB(true);
	return explosion;
}

void Level::playNoteAt(int_t x, int_t y, int_t z, int_t type, int_t data)
{
	int_t tileId = getTile(x, y, z);
	if (tileId > 0 && Tile::tiles[tileId] != nullptr)
		Tile::tiles[tileId]->playBlock(*this, x, y, z, type, data);
}

bool Level::canSeeSky(int_t x, int_t y, int_t z)
{
	return getChunk(x >> 4, z >> 4)->isSkyLit(x & 0xF, y, z & 0xF);
}

int_t Level::getFullBrightness(int_t x, int_t y, int_t z)
{
	if (y < 0)
		return 0;
	if (y >= DEPTH)
		y = DEPTH - 1;
	return getChunk(x >> 4, z >> 4)->getRawBrightness(x & 0xF, y, z & 0xF, 0);
}

int_t Level::getRawBrightness(int_t x, int_t y, int_t z)
{
	return getRawBrightness(x, y, z, true);
}

int_t Level::getRawBrightness(int_t x, int_t y, int_t z, bool neighbors)
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z > MAX_LEVEL_SIZE)
		return 15;

	if (neighbors)
	{
		int_t tile = getTile(x, y, z);
		if (tile == 60 || tile == 44 || tile == 53 || tile == 67)
		{
			int_t brightness = getRawBrightness(x, y + 1, z, false);
			int_t east = getRawBrightness(x + 1, y, z, false);
			int_t west = getRawBrightness(x - 1, y, z, false);
			int_t south = getRawBrightness(x, y, z + 1, false);
			int_t north = getRawBrightness(x, y, z - 1, false);
			if (east > brightness) brightness = east;
			if (west > brightness) brightness = west;
			if (south > brightness) brightness = south;
			if (north > brightness) brightness = north;
			return brightness;
		}
	}

	if (y < 0)
		return 0;

	if (y >= DEPTH)
	{
		int_t l = 15 - skyDarken;
		if (l < 0)
			l = 0;
		return l;
	}

	return getChunk(x >> 4, z >> 4)->getRawBrightness(x & 0xF, y, z & 0xF, skyDarken);
}

bool Level::isSkyLit(int_t x, int_t y, int_t z)
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z > MAX_LEVEL_SIZE)
		return false;
	if (y < 0)
		return false;
	if (y >= DEPTH)
		return true;
	if (!hasChunk(x >> 4, z >> 4))
		return false;
	return getChunk(x >> 4, z >> 4)->isSkyLit(x & 0xF, y, z & 0xF);
}

int_t Level::getHeightmap(int_t x, int_t z)
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z > MAX_LEVEL_SIZE)
		return 0;
	if (!hasChunk(x >> 4, z >> 4))
		return 0;
	return getChunk(x >> 4, z >> 4)->getHeightmap(x & 0xF, z & 0xF);
}


int_t Level::findTopSolidBlock(int_t x, int_t z)
{
	for (int_t y = DEPTH - 1; y > 0; --y)
	{
		const Material &material = getMaterial(x, y, z);
		if (material.isSolid() || material.isLiquid())
			return y + 1;
	}
	return -1;
}

void Level::updateLightIfOtherThan(int_t layer, int_t x, int_t y, int_t z, int_t brightness)
{
	if (dimension->hasCeiling && layer == LightLayer::Sky)
		return;
	if (!hasChunkAt(x, y, z))
		return;

	if (layer == LightLayer::Sky)
	{
		if (isSkyLit(x, y, z))
			brightness = 15;
	}
	else if (layer == LightLayer::Block)
	{
		int_t tile = getTile(x, y, z);
		if (Tile::lightEmission[tile] > brightness)
			brightness = Tile::lightEmission[tile];
	}

	if (getBrightness(layer, x, y, z) != brightness)
		updateLight(layer, x, y, z, x, y, z);
}

int_t Level::getBrightness(int_t layer, int_t x, int_t y, int_t z)
{
	if (y < 0)
		y = 0;
	if (y >= DEPTH)
		y = DEPTH - 1;
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z > MAX_LEVEL_SIZE)
		return LightLayer::surrounding(layer);

	int_t xc = x >> 4;
	int_t zc = z >> 4;
	if (!hasChunk(xc, zc))
		return 0;

	return getChunk(xc, zc)->getBrightness(layer, x & 0xF, y, z & 0xF);
}

void Level::setBrightness(int_t layer, int_t x, int_t y, int_t z, int_t brightness)
{
	if (x < -MAX_LEVEL_SIZE || z < -MAX_LEVEL_SIZE || x >= MAX_LEVEL_SIZE || z > MAX_LEVEL_SIZE)
		return;
	if (y < 0)
		return;
	if (y >= DEPTH)
		return;
	if (!hasChunk(x >> 4, z >> 4))
		return;

	getChunk(x >> 4, z >> 4)->setBrightness(layer, x & 0xF, y, z & 0xF, brightness);

	for (size_t i = 0; i < listeners.size(); ++i)
		listeners[i]->tileChanged(x, y, z);
}

float Level::getBrightness(int_t x, int_t y, int_t z)
{
	return dimension->brightnessRamp[getRawBrightness(x, y, z)];
}

bool Level::isDay()
{
	return skyDarken < 4;
}

HitResult Level::clip(Vec3 &from, Vec3 &to)
{
	return clip(from, to, false);
}

HitResult Level::clip(Vec3 &from, Vec3 &to, bool canPickLiquid)
{
	if (std::isnan(from.x) || std::isnan(from.y) || std::isnan(from.z)) return HitResult();
	if (std::isnan(to.x) || std::isnan(to.y) || std::isnan(to.z)) return HitResult();

	int_t x0 = Mth::floor(to.x);
	int_t y0 = Mth::floor(to.y);
	int_t z0 = Mth::floor(to.z);
	int_t x1 = Mth::floor(from.x);
	int_t y1 = Mth::floor(from.y);
	int_t z1 = Mth::floor(from.z);
	int_t steps = 200;

	while (steps-- >= 0)
	{
		if (std::isnan(from.x) || std::isnan(from.y) || std::isnan(from.z)) return HitResult();
		if (x1 == x0 && y1 == y0 && z1 == z0)
			return HitResult();

		double dx = 999.0;
		double dy = 999.0;
		double dz = 999.0;

		if (x0 > x1) dx = x1 + 1.0;
		if (x0 < x1) dx = x1 + 0.0;
		if (y0 > y1) dy = y1 + 1.0;
		if (y0 < y1) dy = y1 + 0.0;
		if (z0 > z1) dz = z1 + 1.0;
		if (z0 < z1) dz = z1 + 0.0;

		double dx2 = 999.0;
		double dy2 = 999.0;
		double dz2 = 999.0;
		double ddx = to.x - from.x;
		double ddy = to.y - from.y;
		double ddz = to.z - from.z;

		if (dx != 999.0) dx2 = (dx - from.x) / ddx;
		if (dy != 999.0) dy2 = (dy - from.y) / ddy;
		if (dz != 999.0) dz2 = (dz - from.z) / ddz;

		Facing face = Facing::DOWN;
		if (dx2 < dy2 && dx2 < dz2)
		{
			if (x0 > x1)
				face = Facing::WEST;
			else
				face = Facing::EAST;

			from.x = dx;
			from.y += ddy * dx2;
			from.z += ddz * dx2;
		}
		else if (dy2 < dz2)
		{
			if (y0 > y1)
				face = Facing::DOWN;
			else
				face = Facing::UP;

			from.x += ddx * dy2;
			from.y = dy;
			from.z += ddz * dy2;
		}
		else
		{
			if (z0 > z1)
				face = Facing::NORTH;
			else
				face = Facing::SOUTH;

			from.x += ddx * dz2;
			from.y += ddy * dz2;
			from.z = dz;
		}

		Vec3 *newVec = Vec3::newTemp(from.x, from.y, from.z);

		x1 = static_cast<int_t>(newVec->x = Mth::floor(from.x));
		if (face == Facing::EAST)
		{
			x1--;
			newVec->x++;
		}

		y1 = static_cast<int_t>(newVec->y = Mth::floor(from.y));
		if (face == Facing::UP)
		{
			y1--;
			newVec->y++;
		}

		z1 = static_cast<int_t>(newVec->z = Mth::floor(from.z));
		if (face == Facing::SOUTH)
		{
			z1--;
			newVec->z++;
		}

		int_t tile = getTile(x1, y1, z1);
		if (tile > 0)
		{
			Tile *tt = Tile::tiles[tile];
			int_t data = getData(x1, y1, z1);
			if (tt != nullptr && tt->mayPick(data, canPickLiquid))
			{
				HitResult hitResult = tt->clip(*this, x1, y1, z1, from, to);
				if (hitResult.type != HitResult::Type::NONE)
					return hitResult;
			}
		}
	}

	return HitResult();
}


std::shared_ptr<Player> Level::getNearestPlayer(Entity &entity, double radius)
{
	double bestDistance = -1.0;
	std::shared_ptr<Player> nearest;
	for (const auto &player : players)
	{
		if (player == nullptr)
			continue;
		double distance = player->distanceToSqr(entity);
		if ((radius < 0.0 || distance < radius * radius) && (bestDistance == -1.0 || distance < bestDistance))
		{
			bestDistance = distance;
			nearest = player;
		}
	}

	return nearest;
}

std::shared_ptr<Player> Level::getNearestPlayer(double x, double y, double z, double radius)
{
	double bestDistance = -1.0;
	std::shared_ptr<Player> nearest;
	for (const auto &player : players)
	{
		double dx = player->x - x;
		double dy = player->y - y;
		double dz = player->z - z;
		double distance = dx * dx + dy * dy + dz * dz;
		if ((radius < 0.0 || distance < radius * radius) && (bestDistance == -1.0 || distance < bestDistance))
		{
			bestDistance = distance;
			nearest = player;
		}
	}

	return nearest;
}

std::unique_ptr<PathEntity> Level::getPathToEntity(Entity &entity, Entity &target, float distance)
{
	int_t x = Mth::floor(entity.x);
	int_t y = Mth::floor(entity.y);
	int_t z = Mth::floor(entity.z);
	int_t radius = static_cast<int_t>(distance + 16.0f);
	Region region(*this, x - radius, y - radius, z - radius, x + radius, y + radius, z + radius);
	return Pathfinder(region).createEntityPathTo(entity, target, distance);
}

std::unique_ptr<PathEntity> Level::getEntityPathToXYZ(Entity &entity, int_t x, int_t y, int_t z, float distance)
{
	int_t entityX = Mth::floor(entity.x);
	int_t entityY = Mth::floor(entity.y);
	int_t entityZ = Mth::floor(entity.z);
	int_t radius = static_cast<int_t>(distance + 8.0f);
	Region region(*this, entityX - radius, entityY - radius, entityZ - radius, entityX + radius, entityY + radius, entityZ + radius);
	return Pathfinder(region).createEntityPathTo(entity, x, y, z, distance);
}

std::shared_ptr<Entity> Level::getEntityRef(Entity &entity)
{
	for (const auto &candidate : entities)
	{
		if (candidate.get() == &entity)
			return candidate;
	}
	for (const auto &player : players)
	{
		if (player.get() == &entity)
			return std::static_pointer_cast<Entity>(player);
	}
	return nullptr;
}

bool Level::addEntity(std::shared_ptr<Entity> entity)
{
	int_t cx = Mth::floor(entity->x / 16.0);
	int_t cz = Mth::floor(entity->z / 16.0);
	bool isPlayer = entity->isPlayer();

	if (!isPlayer && !hasChunk(cx, cz))
		return false;

	if (isPlayer)
	{
		players.push_back(std::static_pointer_cast<Player>(entity));
		std::cout << "Player count: " << players.size() << '\n';
	}

	getChunk(cx, cz)->addEntity(entity);
	entities.push_back(entity);
	entityAdded(entity);

	return true;
}

bool Level::addWeatherEffect(std::shared_ptr<Entity> entity)
{
	weatherEffects.emplace_back(std::move(entity));
	return true;
}

const std::vector<std::shared_ptr<Entity>> &Level::getWeatherEffects() const
{
	return weatherEffects;
}

void Level::entityAdded(std::shared_ptr<Entity> entity)
{
	for (size_t i = 0; i < listeners.size(); ++i)
		listeners[i]->entityAdded(entity);
}

void Level::entityRemoved(std::shared_ptr<Entity> entity)
{
	for (size_t i = 0; i < listeners.size(); ++i)
		listeners[i]->entityRemoved(entity);
}

void Level::removeEntity(std::shared_ptr<Entity> entity)
{
	if (entity->rider != nullptr)
		entity->rider->ride(nullptr);
	if (entity->riding != nullptr)
		entity->ride(nullptr);
	entity->remove();
	if (entity->isPlayer())
	{
		auto found = std::find_if(players.begin(), players.end(), [&](const auto &player) { return player->entityId == entity->entityId; });
		if (found != players.end())
			players.erase(found);
		updateAllPlayersSleepingFlag();
	}
}

void Level::removeEntityImmediately(std::shared_ptr<Entity> entity)
{
	entity->remove();
	if (entity->isPlayer())
	{
		auto found = std::find_if(players.begin(), players.end(), [&](const auto &player) { return player->entityId == entity->entityId; });
		if (found != players.end())
			players.erase(found);
		updateAllPlayersSleepingFlag();
	}
	int_t cx = entity->xChunk;
	int_t cz = entity->zChunk;
	if (entity->inChunk && hasChunk(cx, cz))
		getChunk(cx, cz)->removeEntity(entity);
	auto found = std::find_if(entities.begin(), entities.end(), [&](const auto &candidate) { return candidate->entityId == entity->entityId; });
	if (found != entities.end())
		entities.erase(found);
	entityRemoved(entity);
}

void Level::addListener(LevelListener &listener)
{
	listeners.push_back(&listener);
}

void Level::removeListener(LevelListener &listener)
{
	auto found = std::find(listeners.begin(), listeners.end(), &listener);
	if (found != listeners.end())
		listeners.erase(found);
}


void Level::playSoundAtEntity(Entity &entity, const jstring &name, float volume, float pitch)
{
	for (size_t i = 0; i < listeners.size(); ++i)
		listeners[i]->playSound(name, entity.x, entity.y - (double)entity.heightOffset, entity.z, volume, pitch);
}


void Level::playSoundEffect(double x, double y, double z, const jstring &name, float volume, float pitch)
{
	for (size_t i = 0; i < listeners.size(); ++i)
		listeners[i]->playSound(name, x, y, z, volume, pitch);
}


void Level::playRecord(const jstring &name, int_t x, int_t y, int_t z)
{
	for (size_t i = 0; i < listeners.size(); ++i)
		listeners[i]->playStreamingMusic(name, x, y, z);
}
void Level::addParticle(const jstring &name, double x, double y, double z, double xa, double ya, double za)
{
	for (size_t i = 0; i < listeners.size(); ++i)
		listeners[i]->addParticle(name, x, y, z, xa, ya, za);
}

void Level::levelEvent(int_t event, int_t x, int_t y, int_t z, int_t data)
{
	levelEvent(nullptr, event, x, y, z, data);
}

void Level::levelEvent(Player *player, int_t event, int_t x, int_t y, int_t z, int_t data)
{
	for (size_t i = 0; i < listeners.size(); ++i)
		listeners[i]->levelEvent(player, event, x, y, z, data);
}

const std::vector<AABB *> &Level::getCubes(Entity &entity, AABB &bb)
{
	boxes.clear();

	int_t x0 = Mth::floor(bb.x0);
	int_t x1 = Mth::floor(bb.x1 + 1.0);
	int_t y0 = Mth::floor(bb.y0);
	int_t y1 = Mth::floor(bb.y1 + 1.0);
	int_t z0 = Mth::floor(bb.z0);
	int_t z1 = Mth::floor(bb.z1 + 1.0);

	for (int_t x = x0; x < x1; x++)
	{
		for (int_t z = z0; z < z1; z++)
		{
			if (!hasChunkAt(x, 64, z))
				continue;
			for (int_t y = y0 - 1; y < y1; y++)
			{
				Tile *tile = Tile::tiles[getTile(x, y, z)];
				if (tile != nullptr)
					tile->addAABBs(*this, x, y, z, bb, boxes);
			}
		}
	}

	double skin = 0.25;
	const auto &overlapEntities = getEntities(&entity, *bb.grow(skin, skin, skin));

	for (size_t i = 0; i < overlapEntities.size(); ++i)
	{
		auto other = overlapEntities.at(i);
		AABB *ebb = other->getCollideBox();
		if (ebb != nullptr && ebb->intersects(bb))
			boxes.push_back(ebb);

		other = overlapEntities.at(i);
		ebb = entity.getCollideAgainstBox(*other);
		if (ebb != nullptr && ebb->intersects(bb))
			boxes.push_back(ebb);
	}

	return boxes;
}

int_t Level::getSkyDarken(float a)
{
	float tod = getTimeOfDay(a);
	float curve = 1.0f - (Mth::cos(tod * Mth::PI * 2.0f) * 2.0f + 0.5f);
	if (curve < 0.0f) curve = 0.0f;
	if (curve > 1.0f) curve = 1.0f;
	curve = 1.0f - curve;
	curve = static_cast<float>(curve * (1.0 - getRainStrength(a) * 5.0f / 16.0));
	curve = static_cast<float>(curve * (1.0 - getThunderStrength(a) * 5.0f / 16.0));
	curve = 1.0f - curve;
	return static_cast<int_t>(curve * 11.0f);
}

static void HSBtoRGB(float hue, float saturation, float brightness, float &r, float &g, float &b)
{
	if (saturation == 0)
	{
		
		r = g = b = brightness;
	}
	else
	{
		hue /= (60.0f / 360.0f);
		int i = static_cast<int>(hue);
		float f = hue - i;
		float p = brightness * (1.0f - saturation);
		float q = brightness * (1.0f - saturation * f);
		float t = brightness * (1.0f - saturation * (1.0f - f));

		switch (i) {
			case 0:
				r = brightness;
				g = t;
				b = p;
				break;
			case 1:
				r = q;
				g = brightness;
				b = p;
				break;
			case 2:
				r = p;
				g = brightness;
				b = t;
				break;
			case 3:
				r = p;
				g = q;
				b = brightness;
				break;
			case 4:
				r = t;
				g = p;
				b = brightness;
				break;
			default:
				r = brightness;
				g = p;
				b = q;
				break;
		}
	}
}

Vec3 *Level::getSkyColor(Entity &entity, float a)
{
	float tod = getTimeOfDay(a);
	float curve = Mth::cos(tod * Mth::PI * 2.0f) * 2.0f + 0.5f;
	if (curve < 0.0f) curve = 0.0f;
	if (curve > 1.0f) curve = 1.0f;

	int_t x = Mth::floor(entity.x);
	int_t z = Mth::floor(entity.z);

	float temperature = static_cast<float>(getBiomeSource().getTemperature(x, z));
	
	float r = 0.0f;
	float g = 0.0f;
	float b = 1.0f;

	temperature /= 3.0f;
	if (temperature < -1.0f) temperature = -1.0f;
	if (temperature > 1.0f) temperature = 1.0f;

	HSBtoRGB(0.6222222f - temperature * 0.05f, 0.5f + temperature * 0.1f, 1.0f, r, g, b);

	r *= curve;
	g *= curve;
	b *= curve;

	float rainStrength = getRainStrength(a);
	if (rainStrength > 0.0f)
	{
		float gray = (r * 0.3f + g * 0.59f + b * 0.11f) * 0.6f;
		float scale = 1.0f - rainStrength * (12.0f / 16.0f);
		r = r * scale + gray * (1.0f - scale);
		g = g * scale + gray * (1.0f - scale);
		b = b * scale + gray * (1.0f - scale);
	}

	float thunderStrength = getThunderStrength(a);
	if (thunderStrength > 0.0f)
	{
		float gray = (r * 0.3f + g * 0.59f + b * 0.11f) * 0.2f;
		float scale = 1.0f - thunderStrength * (12.0f / 16.0f);
		r = r * scale + gray * (1.0f - scale);
		g = g * scale + gray * (1.0f - scale);
		b = b * scale + gray * (1.0f - scale);
	}

	if (lastLightningBolt > 0)
	{
		float flash = lastLightningBolt - a;
		if (flash > 1.0f)
			flash = 1.0f;
		flash *= 0.45f;
		r = r * (1.0f - flash) + 0.8f * flash;
		g = g * (1.0f - flash) + 0.8f * flash;
		b = b * (1.0f - flash) + flash;
	}

	return Vec3::newTemp(r, g, b);
}

float Level::getTimeOfDay(float a)
{
	return dimension->getTimeOfDay(time, a);
}

float Level::getSunAngle(float a)
{
	float tod = getTimeOfDay(a);
	return tod * Mth::PI * 2.0f;
}

Vec3 *Level::getCloudColor(float a)
{
	float tod = getTimeOfDay(a);
	float curve = Mth::cos(tod * Mth::PI * 2.0f) * 2.0f + 0.5f;
	if (curve < 0.0f) curve = 0.0f;
	if (curve > 1.0f) curve = 1.0f;

	float r = ((cloudColor >> 16) & 0xFF) / 255.0f;
	float g = ((cloudColor >> 8) & 0xFF) / 255.0f;
	float b = (cloudColor & 0xFF) / 255.0f;

	float rainStrength = getRainStrength(a);
	if (rainStrength > 0.0f)
	{
		float gray = (r * 0.3f + g * 0.59f + b * 0.11f) * 0.6f;
		float scale = 1.0f - rainStrength * 0.95f;
		r = r * scale + gray * (1.0f - scale);
		g = g * scale + gray * (1.0f - scale);
		b = b * scale + gray * (1.0f - scale);
	}

	r *= curve * 0.9f + 0.1f;
	g *= curve * 0.9f + 0.1f;
	b *= curve * 0.85f + 0.15f;

	float thunderStrength = getThunderStrength(a);
	if (thunderStrength > 0.0f)
	{
		float gray = (r * 0.3f + g * 0.59f + b * 0.11f) * 0.2f;
		float scale = 1.0f - thunderStrength * 0.95f;
		r = r * scale + gray * (1.0f - scale);
		g = g * scale + gray * (1.0f - scale);
		b = b * scale + gray * (1.0f - scale);
	}

	return Vec3::newTemp(r, g, b);
}

Vec3 *Level::getFogColor(float a)
{
	float tod = getTimeOfDay(a);
	return dimension->getFogColor(tod, a);
}

int_t Level::getTopSolidBlock(int_t x, int_t z)
{
	std::shared_ptr<LevelChunk> chunk = getChunk(x >> 4, z >> 4);

	int_t y;
	for (y = DEPTH - 1; getMaterial(x, y, z).blocksMotion() && y > 0; y--);

	x &= 0xF;
	z &= 0xF;

	while (y > 0)
	{
		int_t tile = chunk->getTile(x, y, z);
		if (tile == 0 || (!(Tile::tiles[tile])->material.blocksMotion() && !(Tile::tiles[tile])->material.isLiquid()))
		{
			y--;
			continue;
		}
		return y + 1;
	}
	return -1;
}

int_t Level::getLightDepth(int_t x, int_t z)
{
	return getChunkAt(x, z)->getHeightmap(x & 0xF, z & 0xF);
}

float Level::getStarBrightness(float a)
{
	float tod = getTimeOfDay(a);
	float curve = 1.0f - (Mth::cos(tod * Mth::PI * 2.0f) * 2.0f + 0.75f);
	if (curve < 0.0f) curve = 0.0f;
	if (curve > 1.0f) curve = 1.0f;
	return curve * curve * 0.5f;
}

void Level::addToTickNextTick(int_t x, int_t y, int_t z, int_t tileId)
{
	TickNextTickData entry;
	entry.x = x;
	entry.y = y;
	entry.z = z;
	entry.tileId = tileId;
	ulong_t order = nextTickEntryId++;
	std::memcpy(&entry.order, &order, sizeof(order));

	constexpr int_t chunkRadius = 8;
	if (instaTick)
	{
		if (hasChunksAt(entry.x - chunkRadius, entry.y - chunkRadius, entry.z - chunkRadius, entry.x + chunkRadius, entry.y + chunkRadius, entry.z + chunkRadius))
		{
			int_t tile = getTile(entry.x, entry.y, entry.z);
			if (tile == entry.tileId && tile > 0 && Tile::tiles[tile] != nullptr)
				Tile::tiles[tile]->tick(*this, entry.x, entry.y, entry.z, random);
		}
		return;
	}

	if (!hasChunksAt(x - chunkRadius, y - chunkRadius, z - chunkRadius, x + chunkRadius, y + chunkRadius, z + chunkRadius))
		return;

	if (tileId > 0 && Tile::tiles[tileId] != nullptr)
	{
		ulong_t delay = static_cast<ulong_t>(time) + static_cast<ulong_t>(Tile::tiles[tileId]->getTickDelay());
		std::memcpy(&entry.delay, &delay, sizeof(delay));
	}

	if (tickNextTickSet.find(entry) != tickNextTickSet.end())
		return;

	tickNextTickSet.insert(entry);
	tickNextTickList.insert(entry);
}

void Level::tickEntities()
{
	Profiler::Scope profile(Profiler::Section::EntityTick);
	for (size_t i = 0; i < weatherEffects.size();)
	{
		auto entity = weatherEffects[i];
		entity->tick();
		if (entity->removed)
		{
			if (i >= weatherEffects.size())
				throw std::out_of_range("Weather list index removed during update");
			weatherEffects.erase(weatherEffects.begin() + i);
		}
		else
			++i;
	}

	entities.erase(std::remove_if(entities.begin(), entities.end(), [&](const auto &entity) {
		return std::any_of(entitiesToRemove.begin(), entitiesToRemove.end(), [&](const auto &removed) {
			return entity->entityId == removed->entityId;
		});
	}), entities.end());
	for (size_t i = 0; i < entitiesToRemove.size(); ++i)
	{
		auto entity = entitiesToRemove[i];
		if (entity->inChunk && hasChunk(entity->xChunk, entity->zChunk))
			getChunk(entity->xChunk, entity->zChunk)->removeEntity(entity);
	}
	for (size_t i = 0; i < entitiesToRemove.size(); ++i)
		entityRemoved(entitiesToRemove[i]);
	entitiesToRemove.clear();

	
	for (size_t i = 0; i < entities.size();)
	{
		auto entity = entities[i];
		if (entity->riding != nullptr)
		{
			if (!entity->riding->removed && entity->riding->rider == entity)
			{
				++i;
				continue;
			}
			entity->riding->rider = nullptr;
			entity->riding = nullptr;
		}
		if (!entity->removed)
			tick(entity);
		if (entity->removed)
		{
			if (entity->inChunk && hasChunk(entity->xChunk, entity->zChunk))
				getChunk(entity->xChunk, entity->zChunk)->removeEntity(entity);
			if (i >= entities.size())
				throw std::out_of_range("Entity list index removed during update");
			entities.erase(entities.begin() + i);
			entityRemoved(entity);
		}
		else
			++i;
	}

	tickingTileEntities = true;
	for (size_t i = 0; i < tileEntityList.size();)
	{
		auto tileEntity = tileEntityList[i];
		if (!tileEntity->isRemoved())
			tileEntity->tick();
		if (tileEntity->isRemoved())
		{
			tileEntityList.erase(tileEntityList.begin() + i);
			auto chunk = getChunk(tileEntity->x >> 4, tileEntity->z >> 4);
			if (chunk != nullptr)
				chunk->removeTileEntity(tileEntity->x & 15, tileEntity->y, tileEntity->z & 15);
		}
		else
			++i;
	}
	tickingTileEntities = false;
	for (size_t i = 0; i < pendingTileEntities.size(); ++i)
	{
		auto tileEntity = pendingTileEntities[i];
		if (tileEntity->isRemoved())
			continue;
		if (std::find(tileEntityList.begin(), tileEntityList.end(), tileEntity) == tileEntityList.end())
			tileEntityList.push_back(tileEntity);
		auto chunk = getChunk(tileEntity->x >> 4, tileEntity->z >> 4);
		if (chunk != nullptr)
			chunk->setTileEntity(tileEntity->x & 15, tileEntity->y, tileEntity->z & 15, tileEntity);
		sendTileUpdated(tileEntity->x, tileEntity->y, tileEntity->z);
	}
	pendingTileEntities.clear();
}

void Level::tick(std::shared_ptr<Entity> entity)
{
	tick(entity, true);
}

void Level::tick(std::shared_ptr<Entity> entity, bool ai)
{
	int_t xt = Mth::floor(entity->x);
	int_t zt = Mth::floor(entity->z);

	byte_t rad = 32;
	if (ai && !hasChunksAt(xt - rad, 0, zt - rad, xt + rad, DEPTH, zt + rad))
		return;

	
	entity->xOld = entity->x;
	entity->yOld = entity->y;
	entity->zOld = entity->z;
	entity->yRotO = entity->yRot;
	entity->xRotO = entity->xRot;

	if (ai && entity->inChunk)
	{
		if (entity->riding != nullptr)
			entity->rideTick();
		else
			entity->tick();
	}
	
	

	
	if (std::isnan(entity->x) || std::isinf(entity->x))
		entity->x = entity->xOld;
	if (std::isnan(entity->y) || std::isinf(entity->y))
		entity->y = entity->yOld;
	if (std::isnan(entity->z) || std::isinf(entity->z))
		entity->z = entity->zOld;
	if (std::isnan(entity->yRot) || std::isinf(entity->yRot))
		entity->yRot = entity->yRotO;
	if (std::isnan(entity->xRot) || std::isinf(entity->xRot))
		entity->xRot = entity->xRotO;

	
	int_t xc = Mth::floor(entity->x / 16.0);
	int_t yc = Mth::floor(entity->y / 16.0);
	int_t zc = Mth::floor(entity->z / 16.0);

	if (!entity->inChunk || entity->xChunk != xc || entity->yChunk != yc || entity->zChunk != zc)
	{
		if (entity->inChunk && hasChunk(entity->xChunk, entity->zChunk))
			getChunk(entity->xChunk, entity->zChunk)->removeEntity(entity);

		if (hasChunk(xc, zc))
		{
			entity->inChunk = true;
			getChunk(xc, zc)->addEntity(entity);
		}
		else
		{
			entity->inChunk = false;
		}
	}

	
	if (ai && entity->inChunk && entity->rider != nullptr)
	{
		if (entity->rider->removed || entity->rider->riding != entity)
		{
			entity->rider->riding = nullptr;
			entity->rider = nullptr;
		}
		else
		{
			tick(entity->rider);
		}
	}
}

bool Level::isUnobstructed(AABB &bb)
{
	auto &overlapEntities = getEntities(nullptr, bb);
	for (auto &entity : overlapEntities)
	{
		if (!entity->removed && entity->blocksBuilding)
			return false;
	}
	return true;
}

bool Level::containsAnyLiquid(AABB &bb)
{
	int_t x0 = Mth::floor(bb.x0);
	int_t x1 = Mth::floor(bb.x1 + 1.0);
	int_t y0 = Mth::floor(bb.y0);
	int_t y1 = Mth::floor(bb.y1 + 1.0);
	int_t z0 = Mth::floor(bb.z0);
	int_t z1 = Mth::floor(bb.z1 + 1.0);

	if (!hasChunksAt(x0, y0, z0, x1, y1, z1))
		return false;

	for (int_t x = x0; x < x1; ++x)
		for (int_t y = y0; y < y1; ++y)
			for (int_t z = z0; z < z1; ++z)
			{
				int_t t = getTile(x, y, z);
				if (t > 0 && Tile::tiles[t] != nullptr && Tile::tiles[t]->material.isLiquid())
					return true;
			}

	return false;
}

bool Level::isMaterialInBB(AABB &bb, const Material &material)
{
	int_t x0 = Mth::floor(bb.x0);
	int_t x1 = Mth::floor(bb.x1 + 1.0);
	int_t y0 = Mth::floor(bb.y0);
	int_t y1 = Mth::floor(bb.y1 + 1.0);
	int_t z0 = Mth::floor(bb.z0);
	int_t z1 = Mth::floor(bb.z1 + 1.0);

	for (int_t x = x0; x < x1; ++x)
		for (int_t y = y0; y < y1; ++y)
			for (int_t z = z0; z < z1; ++z)
			{
				int_t t = getTile(x, y, z);
				if (t > 0 && Tile::tiles[t] != nullptr && &Tile::tiles[t]->material == &material)
					return true;
			}

	return false;
}

bool Level::handleMaterialAcceleration(AABB &bb, const Material &material, Entity &entity)
{
	int_t x0 = Mth::floor(bb.x0);
	int_t x1 = Mth::floor(bb.x1 + 1.0);
	int_t y0 = Mth::floor(bb.y0);
	int_t y1 = Mth::floor(bb.y1 + 1.0);
	int_t z0 = Mth::floor(bb.z0);
	int_t z1 = Mth::floor(bb.z1 + 1.0);

	if (!hasChunksAt(x0, y0, z0, x1, y1, z1))
		return false;

	bool found = false;
	double vx = 0.0, vy = 0.0, vz = 0.0;

	for (int_t x = x0; x < x1; ++x)
	{
		for (int_t y = y0; y < y1; ++y)
		{
			for (int_t z = z0; z < z1; ++z)
			{
				int_t t = getTile(x, y, z);
				if (t > 0 && Tile::tiles[t] != nullptr && &Tile::tiles[t]->material == &material)
				{
					
					int_t meta = getData(x, y, z);
					float pct = static_cast<float>((meta >= 8 ? 0 : meta) + 1) / 9.0f;
					double surface = static_cast<double>(y + 1) - static_cast<double>(pct);
					if (static_cast<double>(y1) >= surface)
					{
					found = true;
					Vec3 flow = static_cast<LiquidTile *>(Tile::tiles[t])->getFlowVector(*this, x, y, z);
					vx += flow.x;
					vy += flow.y;
					vz += flow.z;
					}
				}
			}
		}
	}

	if (vx * vx + vy * vy + vz * vz > 0.0)
	{
		double len = Mth::sqrt(vx * vx + vy * vy + vz * vz);
		entity.xd += (vx / len) * 0.004;
		entity.yd += (vy / len) * 0.004;
		entity.zd += (vz / len) * 0.004;
	}

	return found;
}

bool Level::containsFireTile(AABB &bb)
{
	int_t x0 = Mth::floor(bb.x0);
	int_t x1 = Mth::floor(bb.x1 + 1.0);
	int_t y0 = Mth::floor(bb.y0);
	int_t y1 = Mth::floor(bb.y1 + 1.0);
	int_t z0 = Mth::floor(bb.z0);
	int_t z1 = Mth::floor(bb.z1 + 1.0);

	if (hasChunksAt(x0, y0, z0, x1, y1, z1))
	{
		for (int_t x = x0; x < x1; ++x)
		{
			for (int_t y = y0; y < y1; ++y)
			{
				for (int_t z = z0; z < z1; ++z)
				{
					int_t tile = getTile(x, y, z);
					if (tile == Tile::fire.id || tile == Tile::lava.id || tile == Tile::calmLava.id)
						return true;
				}
			}
		}
	}

	return false;
}

void Level::extinguishFire(int_t x, int_t y, int_t z, Facing f)
{
	if (f == Facing::DOWN)
		y--;
	if (f == Facing::UP)
		y++;
	if (f == Facing::NORTH)
		z--;
	if (f == Facing::SOUTH)
		z++;
	if (f == Facing::WEST)
		x--;
	if (f == Facing::EAST)
		x++;

	if (getTile(x, y, z) == Tile::fire.id)
	{
		playSoundEffect(static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5, static_cast<double>(z) + 0.5,
			u"random.fizz", 0.5f, 2.6f + (random.nextFloat() - random.nextFloat()) * 0.8f);
		setTile(x, y, z, 0);
	}
}

jstring Level::gatherStats()
{
	return u"All: " + String::toString((int)entities.size());
}

jstring Level::gatherChunkSourceStats()
{
	return chunkSource->gatherStats();
}

std::shared_ptr<TileEntity> Level::getTileEntity(int_t x, int_t y, int_t z)
{
	std::shared_ptr<LevelChunk> chunk = getChunk(x >> 4, z >> 4);
	return (chunk != nullptr) ? chunk->getTileEntity(x & 0xF, y, z & 0xF) : nullptr;
}

void Level::setTileEntity(int_t x, int_t y, int_t z, std::shared_ptr<TileEntity> tileEntity)
{
	if (tileEntity->isRemoved())
		return;
	if (tickingTileEntities)
	{
		tileEntity->x = x;
		tileEntity->y = y;
		tileEntity->z = z;
		pendingTileEntities.push_back(tileEntity);
	}
	else
	{
		tileEntityList.push_back(tileEntity);
		auto chunk = getChunk(x >> 4, z >> 4);
		if (chunk != nullptr)
			chunk->setTileEntity(x & 15, y, z & 15, tileEntity);
	}
}

void Level::removeTileEntity(int_t x, int_t y, int_t z)
{
	auto tileEntity = getTileEntity(x, y, z);
	if (tileEntity != nullptr && tickingTileEntities)
	{
		tileEntity->setRemoved();
		return;
	}
	if (tileEntity != nullptr)
	{
		auto found = std::find(tileEntityList.begin(), tileEntityList.end(), tileEntity);
		if (found != tileEntityList.end())
			tileEntityList.erase(found);
	}
	auto chunk = getChunk(x >> 4, z >> 4);
	if (chunk != nullptr)
		chunk->removeTileEntity(x & 15, y, z & 15);
}

void Level::addTileEntities(const std::vector<std::shared_ptr<TileEntity>> &tileEntities)
{
	auto &destination = tickingTileEntities ? pendingTileEntities : tileEntityList;
	destination.insert(destination.end(), tileEntities.begin(), tileEntities.end());
}

bool Level::isSolidTile(int_t x, int_t y, int_t z)
{
	Tile *tile = Tile::tiles[getTile(x, y, z)];
	return (tile == nullptr) ? false : tile->isSolidRender();
}
void Level::forceSave(std::shared_ptr<ProgressListener> progressRenderer)
{
	save(true, progressRenderer);
}

int_t Level::getLightsToUpdate()
{
	return static_cast<int_t>(lightUpdates.size());
}

bool Level::updateLights()
{
	Profiler::Scope profile(Profiler::Section::Lighting);
	if (maxRecurse >= 50)
		return false;
	LevelLightRecursion recursion(maxRecurse);

	int_t limit = 500;
	while (!lightUpdates.empty())
	{
		if (--limit <= 0)
		{
			return true;
		}

		LightUpdate update = lightUpdates.back();
		lightUpdates.pop_back();

		update.update(*this);
	}

	return false;
}

void Level::updateLight(int_t layer, int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1)
{
	updateLight(layer, x0, y0, z0, x1, y1, z1, true);
}

void Level::updateLight(int_t layer, int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1, bool checkExpansion)
{
	if (dimension->hasCeiling && layer == LightLayer::Sky)
		return;

	LevelLightRecursion recursion(maxLoop);
	if (maxLoop == 50)
		return;

	int_t xm = (x1 + x0) / 2;
	int_t zm = (z1 + z0) / 2;
	if (!hasChunkAt(xm, 64, zm))
	{
		return;
	}

	if (getChunkAt(xm, zm)->isEmpty())
	{
		return;
	}

	int_t updates = static_cast<int_t>(lightUpdates.size());
	if (checkExpansion)
	{
		
		int_t checkLimit = 5;
		if (checkLimit > updates)
			checkLimit = updates;

		for (int_t i = 0; i < checkLimit; i++)
		{
			LightUpdate &other = lightUpdates[lightUpdates.size() - i - 1];
			if (other.layer == layer && other.expandToContain(x0, y0, z0, x1, y1, z1))
			{
				return;
			}
		}
	}

	
	lightUpdates.emplace_back(layer, x0, y0, z0, x1, y1, z1);

	int_t limit = 1000000;
	if (lightUpdates.size() > limit)
	{
		std::cout << "More than " << limit << " updates, aborting lighting updates\n";
		lightUpdates.clear();
	}

}

void Level::setSimulationSeed(long_t seed)
{
	random.setSeed(seed);
	randValue = random.nextInt();
	delayUntilNextMoodSound = random.nextInt(12000);
	raining = false;
	thundering = false;
	rainTime = thunderTime = 0;
	previousRainingStrength = rainingStrength = 0.0f;
	previousThunderingStrength = thunderingStrength = 0.0f;
}

void Level::updateSkyBrightness()
{
	int_t newDarken = getSkyDarken(1.0f);
	if (newDarken != skyDarken)
		skyDarken = newDarken;
}

namespace
{
	int_t javaIntFromBits(uint_t value)
	{
		int_t result;
		std::memcpy(&result, &value, sizeof(result));
		return result;
	}

	uint_t javaIntBits(int_t value)
	{
		uint_t result;
		std::memcpy(&result, &value, sizeof(result));
		return result;
	}

	int_t javaIntAdd(int_t a, int_t b)
	{
		return javaIntFromBits(javaIntBits(a) + javaIntBits(b));
	}

	int_t javaIntSubtract(int_t a, int_t b)
	{
		return javaIntFromBits(javaIntBits(a) - javaIntBits(b));
	}

	int_t javaIntMultiply(int_t a, int_t b)
	{
		return javaIntFromBits(javaIntBits(a) * javaIntBits(b));
	}

	struct NaturalSpawnChunk
	{
		int_t x;
		int_t z;

		bool operator==(const NaturalSpawnChunk &other) const
		{
			return x == other.x && z == other.z;
		}
	};

	class JavaNaturalSpawnChunkSet
	{
	private:
		struct Entry
		{
			uint_t hash;
			NaturalSpawnChunk value;
			int_t next;
		};

		std::vector<Entry> entries;
		std::vector<int_t> bucketHeads = std::vector<int_t>(16, -1);
		int_t resizeThreshold = 12;

		static uint_t hash(const NaturalSpawnChunk &chunk)
		{
			uint_t value = (chunk.x < 0 ? 0x80000000u : 0u)
				| ((javaIntBits(chunk.x) & 0x7fffu) << 16)
				| (chunk.z < 0 ? 0x8000u : 0u)
				| (javaIntBits(chunk.z) & 0x7fffu);
			value ^= (value >> 20) ^ (value >> 12);
			return value ^ (value >> 7) ^ (value >> 4);
		}

		int_t findEntry(const NaturalSpawnChunk &chunk) const
		{
			uint_t chunkHash = hash(chunk);
			int_t entryIndex = bucketHeads[chunkHash & (bucketHeads.size() - 1)];
			while (entryIndex >= 0)
			{
				const Entry &entry = entries[static_cast<size_t>(entryIndex)];
				if (entry.hash == chunkHash && entry.value == chunk)
					return entryIndex;
				entryIndex = entry.next;
			}
			return -1;
		}

		void resize(int_t capacity)
		{
			std::vector<int_t> newBucketHeads(static_cast<size_t>(capacity), -1);
			for (int_t bucketHead : bucketHeads)
			{
				int_t entryIndex = bucketHead;
				while (entryIndex >= 0)
				{
					Entry &entry = entries[static_cast<size_t>(entryIndex)];
					int_t next = entry.next;
					int_t bucket = static_cast<int_t>(entry.hash & (newBucketHeads.size() - 1));
					entry.next = newBucketHeads[static_cast<size_t>(bucket)];
					newBucketHeads[static_cast<size_t>(bucket)] = entryIndex;
					entryIndex = next;
				}
			}
			bucketHeads = std::move(newBucketHeads);
			resizeThreshold = capacity * 3 / 4;
		}

	public:
		void clear()
		{
			entries.clear();
			std::fill(bucketHeads.begin(), bucketHeads.end(), -1);
		}

		void add(int_t x, int_t z)
		{
			NaturalSpawnChunk chunk{x, z};
			if (findEntry(chunk) >= 0)
				return;

			uint_t chunkHash = hash(chunk);
			int_t bucket = static_cast<int_t>(chunkHash & (bucketHeads.size() - 1));
			int_t sizeBefore = static_cast<int_t>(entries.size());
			entries.push_back({chunkHash, chunk, bucketHeads[static_cast<size_t>(bucket)]});
			bucketHeads[static_cast<size_t>(bucket)] = sizeBefore;
			if (sizeBefore >= resizeThreshold)
				resize(static_cast<int_t>(bucketHeads.size() * 2));
		}

		int_t size() const
		{
			return static_cast<int_t>(entries.size());
		}

		std::vector<NaturalSpawnChunk> orderedValues() const
		{
			std::vector<NaturalSpawnChunk> values;
			values.reserve(entries.size());
			for (int_t bucketHead : bucketHeads)
			{
				int_t entryIndex = bucketHead;
				while (entryIndex >= 0)
				{
					const Entry &entry = entries[static_cast<size_t>(entryIndex)];
					values.push_back(entry.value);
					entryIndex = entry.next;
				}
			}
			return values;
		}
	};

	enum class NaturalCreatureCategory
	{
		Monster,
		Creature,
		WaterCreature,
	};

	bool isCreatureEntity(Entity &entity)
	{
		return dynamic_cast<Animal *>(&entity) != nullptr && dynamic_cast<Squid *>(&entity) == nullptr;
	}

	bool isWaterCreatureEntity(Entity &entity)
	{
		return dynamic_cast<Squid *>(&entity) != nullptr;
	}

	bool isMonsterEntity(Entity &entity)
	{
		return dynamic_cast<Monster *>(&entity) != nullptr || dynamic_cast<Slime *>(&entity) != nullptr
			|| dynamic_cast<Ghast *>(&entity) != nullptr;
	}

	struct NaturalCreatureType
	{
		NaturalCreatureCategory category;
		int_t maxNumberOfCreature;
		const Material *creatureMaterial;
		bool peacefulCreature;
		bool (*countPredicate)(Entity &);
	};

	const std::array<NaturalCreatureType, 3> &getNaturalCreatureTypes()
	{
		static const std::array<NaturalCreatureType, 3> types = {{
			{ NaturalCreatureCategory::Monster, 70, &Material::air, false, isMonsterEntity },
			{ NaturalCreatureCategory::Creature, 15, &Material::air, true, isCreatureEntity },
			{ NaturalCreatureCategory::WaterCreature, 5, &Material::water, true, isWaterCreatureEntity },
		}};
		return types;
	}

	enum class NaturalSpawnEntity
	{
		Spider,
		Zombie,
		Skeleton,
		Creeper,
		Slime,
		Sheep,
		Pig,
		Chicken,
		Cow,
		Wolf,
		Squid,
		Ghast,
		PigZombie,
	};

	struct NaturalSpawnEntry
	{
		NaturalSpawnEntity entityClass;
		int_t spawnRarityRate;
	};

	const std::vector<NaturalSpawnEntry> &getNaturalSpawnList(BiomeId biome, NaturalCreatureCategory category)
	{
		static const std::vector<NaturalSpawnEntry> empty;
		static const std::vector<NaturalSpawnEntry> monsters = {
			{ NaturalSpawnEntity::Spider, 10 },
			{ NaturalSpawnEntity::Zombie, 10 },
			{ NaturalSpawnEntity::Skeleton, 10 },
			{ NaturalSpawnEntity::Creeper, 10 },
			{ NaturalSpawnEntity::Slime, 10 },
		};
		static const std::vector<NaturalSpawnEntry> creatures = {
			{ NaturalSpawnEntity::Sheep, 12 },
			{ NaturalSpawnEntity::Pig, 10 },
			{ NaturalSpawnEntity::Chicken, 10 },
			{ NaturalSpawnEntity::Cow, 8 },
		};
		static const std::vector<NaturalSpawnEntry> forestCreatures = {
			{ NaturalSpawnEntity::Sheep, 12 },
			{ NaturalSpawnEntity::Pig, 10 },
			{ NaturalSpawnEntity::Chicken, 10 },
			{ NaturalSpawnEntity::Cow, 8 },
			{ NaturalSpawnEntity::Wolf, 2 },
		};
		static const std::vector<NaturalSpawnEntry> waterCreatures = {
			{ NaturalSpawnEntity::Squid, 10 },
		};
		static const std::vector<NaturalSpawnEntry> hellMonsters = {
			{ NaturalSpawnEntity::Ghast, 10 },
			{ NaturalSpawnEntity::PigZombie, 10 },
		};

		if (biome == BiomeId::Hell)
			return category == NaturalCreatureCategory::Monster ? hellMonsters : empty;
		if (category == NaturalCreatureCategory::Monster)
			return monsters;
		if (category == NaturalCreatureCategory::WaterCreature)
			return waterCreatures;
		if (biome == BiomeId::Forest || biome == BiomeId::Taiga)
			return forestCreatures;
		return creatures;
	}

	std::shared_ptr<Mob> createNaturalSpawnEntity(Level &level, NaturalSpawnEntity entityClass)
	{
		switch (entityClass)
		{
		case NaturalSpawnEntity::Spider: return std::make_shared<Spider>(level);
		case NaturalSpawnEntity::Zombie: return std::make_shared<Zombie>(level);
		case NaturalSpawnEntity::Skeleton: return std::make_shared<Skeleton>(level);
		case NaturalSpawnEntity::Creeper: return std::make_shared<Creeper>(level);
		case NaturalSpawnEntity::Slime: return std::make_shared<Slime>(level);
		case NaturalSpawnEntity::Sheep: return std::make_shared<Sheep>(level);
		case NaturalSpawnEntity::Pig: return std::make_shared<Pig>(level);
		case NaturalSpawnEntity::Chicken: return std::make_shared<Chicken>(level);
		case NaturalSpawnEntity::Cow: return std::make_shared<Cow>(level);
		case NaturalSpawnEntity::Wolf: return std::make_shared<Wolf>(level);
		case NaturalSpawnEntity::Squid: return std::make_shared<Squid>(level);
		case NaturalSpawnEntity::Ghast: return std::make_shared<Ghast>(level);
		case NaturalSpawnEntity::PigZombie: return std::make_shared<PigZombie>(level);
		}
		return nullptr;
	}

	bool canCreatureTypeSpawnAtLocation(Level &level, bool waterCreature, int_t x, int_t y, int_t z)
	{
		if (waterCreature)
			return level.getMaterial(x, y, z).isLiquid() && !level.isBlockNormalCube(x, y + 1, z);
		return level.isBlockNormalCube(x, y - 1, z) && !level.isBlockNormalCube(x, y, z)
			&& !level.getMaterial(x, y, z).isLiquid() && !level.isBlockNormalCube(x, y + 1, z);
	}

	void naturalSpawnSpecificInit(const std::shared_ptr<Mob> &entity, Level &level, float x, float y, float z)
	{
		std::shared_ptr<Spider> spider = std::dynamic_pointer_cast<Spider>(entity);
		if (spider != nullptr)
		{
			if (level.random.nextInt(100) == 0)
			{
				std::shared_ptr<Skeleton> skeleton = std::make_shared<Skeleton>(level);
				skeleton->moveTo(x, y, z, entity->yRot, 0.0f);
				level.addEntity(skeleton);
				skeleton->ride(entity);
			}
		}
		else
		{
			std::shared_ptr<Sheep> sheep = std::dynamic_pointer_cast<Sheep>(entity);
			if (sheep != nullptr)
				sheep->setFleeceColor(Sheep::getRandomFleeceColor(level.random));
		}
	}

	
	int_t performNaturalSpawningImpl(Level &level, bool spawnHostileMobs, bool spawnPeacefulMobs)
	{
		if (!spawnHostileMobs && !spawnPeacefulMobs)
			return 0;

		static JavaNaturalSpawnChunkSet eligibleChunksForSpawning;
		eligibleChunksForSpawning.clear();
		for (const auto &player : level.players)
		{
			int_t playerChunkX = Mth::floor(player->x / 16.0);
			int_t playerChunkZ = Mth::floor(player->z / 16.0);
			constexpr int_t chunkRadius = 8;
			for (int_t x = -chunkRadius; x <= chunkRadius; ++x)
			{
				for (int_t z = -chunkRadius; z <= chunkRadius; ++z)
				{
					eligibleChunksForSpawning.add(javaIntAdd(x, playerChunkX), javaIntAdd(z, playerChunkZ));
				}
			}
		}

		int_t totalSpawns = 0;
		std::vector<NaturalSpawnChunk> orderedChunks = eligibleChunksForSpawning.orderedValues();
		for (const NaturalCreatureType &creatureType : getNaturalCreatureTypes())
		{
			if ((creatureType.peacefulCreature && !spawnPeacefulMobs)
				|| (!creatureType.peacefulCreature && !spawnHostileMobs)
				|| level.countConditionOf(creatureType.countPredicate) > javaIntMultiply(
					creatureType.maxNumberOfCreature, eligibleChunksForSpawning.size()) / 256)
				continue;

			for (const NaturalSpawnChunk &chunk : orderedChunks)
			{
				int_t chunkBlockX = javaIntMultiply(chunk.x, 16);
				int_t chunkBlockZ = javaIntMultiply(chunk.z, 16);
				BiomeId biome = level.getBiomeSource().getBiome(chunkBlockX, chunkBlockZ);
				const std::vector<NaturalSpawnEntry> &spawnList = getNaturalSpawnList(biome, creatureType.category);
				if (spawnList.empty())
					continue;

				int_t totalWeight = 0;
				for (const NaturalSpawnEntry &entry : spawnList)
					totalWeight = javaIntAdd(totalWeight, entry.spawnRarityRate);
				int_t selectedWeight = level.random.nextInt(totalWeight);
				const NaturalSpawnEntry *selectedEntry = &spawnList.front();
				for (const NaturalSpawnEntry &entry : spawnList)
				{
					selectedWeight = javaIntSubtract(selectedWeight, entry.spawnRarityRate);
					if (selectedWeight < 0)
					{
						selectedEntry = &entry;
						break;
					}
				}

				int_t baseX = javaIntAdd(chunkBlockX, level.random.nextInt(16));
				int_t baseY = level.random.nextInt(128);
				int_t baseZ = javaIntAdd(chunkBlockZ, level.random.nextInt(16));
				if (level.isBlockNormalCube(baseX, baseY, baseZ)
					|| &level.getMaterial(baseX, baseY, baseZ) != creatureType.creatureMaterial)
					continue;

				int_t packSize = 0;
				bool nextChunk = false;
				for (int_t outer = 0; outer < 3 && !nextChunk; ++outer)
				{
					int_t x = baseX;
					int_t y = baseY;
					int_t z = baseZ;
					constexpr int_t spawnDistance = 6;
					for (int_t inner = 0; inner < 4; ++inner)
					{
						x = javaIntAdd(x, javaIntSubtract(level.random.nextInt(spawnDistance), level.random.nextInt(spawnDistance)));
						y = javaIntAdd(y, javaIntSubtract(level.random.nextInt(1), level.random.nextInt(1)));
						z = javaIntAdd(z, javaIntSubtract(level.random.nextInt(spawnDistance), level.random.nextInt(spawnDistance)));
						bool waterCreature = creatureType.creatureMaterial == &Material::water;
						if (!canCreatureTypeSpawnAtLocation(level, waterCreature, x, y, z))
							continue;

						float spawnX = static_cast<float>(x) + 0.5f;
						float spawnY = static_cast<float>(y);
						float spawnZ = static_cast<float>(z) + 0.5f;
						if (level.getNearestPlayer(spawnX, spawnY, spawnZ, 24.0) != nullptr)
							continue;

						float dx = spawnX - static_cast<float>(level.xSpawn);
						float dy = spawnY - static_cast<float>(level.ySpawn);
						float dz = spawnZ - static_cast<float>(level.zSpawn);
						float distanceToSpawn = dx * dx + dy * dy + dz * dz;
						if (distanceToSpawn < 576.0f)
							continue;

						std::shared_ptr<Mob> entity = createNaturalSpawnEntity(level, selectedEntry->entityClass);
						if (entity == nullptr)
							return totalSpawns;
						entity->moveTo(spawnX, spawnY, spawnZ, level.random.nextFloat() * 360.0f, 0.0f);
						if (entity->canSpawn())
						{
							packSize = javaIntAdd(packSize, 1);
							level.addEntity(entity);
							naturalSpawnSpecificInit(entity, level, spawnX, spawnY, spawnZ);
							if (packSize >= entity->getMaxSpawnClusterSize())
							{
								nextChunk = true;
								break;
							}
						}
						totalSpawns = javaIntAdd(totalSpawns, packSize);
					}
				}
			}
		}

		return totalSpawns;
	}
}

int_t SpawnerAnimals::performSpawning(Level &level, bool spawnHostileMobs, bool spawnPeacefulMobs)
{
	return performNaturalSpawningImpl(level, spawnHostileMobs, spawnPeacefulMobs);
}

void Level::setSpawnSettings(bool spawnEnemies, bool spawnFriendlies)
{
	this->spawnEnemies = spawnEnemies;
	this->spawnFriendlies = spawnFriendlies;
}

void Level::tick()
{
	updateWeather();

	if (allPlayersSleeping && !isOnline)
	{
		if (isAllPlayersFullyAsleep())
		{
			
			
			bool interrupted = false;
			if (spawnEnemies && difficulty >= 1)
				interrupted = performSleepSpawning();

			if (!interrupted)
			{
				long_t newTime = time + 24000LL;
				setTime(newTime - newTime % 24000LL);
				wakeUpAllPlayers();
			}
		}
	}

	if (!isOnline)
		SpawnerAnimals::performSpawning(*this, spawnEnemies, spawnFriendlies);
	chunkSource->tick();

	int_t newDarken = getSkyDarken(1.0f);
	if (newDarken != skyDarken)
	{
		skyDarken = newDarken;
		for (size_t i = 0; i < listeners.size(); ++i)
			listeners[i]->skyColorChanged();
	}

	time++;
	if (time % saveInterval == 0)
		save(false, nullptr);

	tickPendingTicks(false);
	tickTiles();
}

void Level::updateAllPlayersSleepingFlag()
{
	allPlayersSleeping = !players.empty();
	for (const auto &p : players)
	{
		if (!p->isPlayerSleeping())
		{
			allPlayersSleeping = false;
			break;
		}
	}
}

void Level::wakeUpAllPlayers()
{
	allPlayersSleeping = false;
	for (const auto &p : players)
	{
		if (p->isPlayerSleeping())
			p->wakeUpPlayer(false, false, true);
	}
	stopPrecipitation();
}




bool Level::performSleepSpawning()
{
	bool spawned = false;

	for (const auto &player : players)
	{
		bool interrupted = false;
		for (int_t attempt = 0; attempt < 20 && !interrupted; attempt++)
		{
			int_t sx = Mth::floor(player->x) + random.nextInt(32) - random.nextInt(32);
			int_t sz = Mth::floor(player->z) + random.nextInt(32) - random.nextInt(32);
			int_t sy = Mth::floor(player->y) + random.nextInt(16) - random.nextInt(16);
			if (sy < 1)
				sy = 1;
			else if (sy > 128)
				sy = 128;

			int_t typeIndex = random.nextInt(3);
			int_t y = sy;

			while (y > 2 && !isBlockNormalCube(sx, y - 1, sz))
				y--;

			while (!canCreatureTypeSpawnAtLocation(*this, false, sx, y, sz) && y < sy + 16 && y < 128)
				y++;

			if (y >= sy + 16 || y >= 128)
				continue;

			float spawnX = sx + 0.5f;
			float spawnY = static_cast<float>(y);
			float spawnZ = sz + 0.5f;

			std::shared_ptr<Mob> mob;
			if (typeIndex == 0)
				mob = std::make_shared<Spider>(*this);
			else if (typeIndex == 1)
				mob = std::make_shared<Zombie>(*this);
			else
				mob = std::make_shared<Skeleton>(*this);

			mob->moveTo(spawnX, spawnY, spawnZ, random.nextFloat() * 360.0f, 0.0f);
			if (!mob->canSpawn())
				continue;

			std::unique_ptr<PathEntity> path = getPathToEntity(*mob, *player, 32.0f);
			if (path == nullptr || path->pathLength <= 1)
				continue;

			const PathPoint *end = path->getEndPoint();
			if (end == nullptr)
				continue;
			if (Mth::abs(static_cast<float>(end->xCoord - player->x)) >= 1.5f
				|| Mth::abs(static_cast<float>(end->zCoord - player->z)) >= 1.5f
				|| Mth::abs(static_cast<float>(end->yCoord - player->y)) >= 1.5f)
				continue;

			int_t spotX = sx;
			int_t spotY = y + 1;
			int_t spotZ = sz;
			BedTile::getNearestEmptySpot(*this, Mth::floor(player->x), Mth::floor(player->y),
				Mth::floor(player->z), 1, spotX, spotY, spotZ);

			mob->moveTo(spotX + 0.5f, static_cast<float>(spotY), spotZ + 0.5f, 0.0f, 0.0f);
			addEntity(mob);
			player->wakeUpPlayer(true, false, false);
			mob->playAmbientSound();
			spawned = true;
			interrupted = true;
		}
	}

	return spawned;
}

bool Level::isAllPlayersFullyAsleep()
{
	if (allPlayersSleeping && !isOnline)
	{
		for (const auto &p : players)
		{
			if (!p->isPlayerFullyAsleep())
				return false;
		}
		return true;
	}
	return false;
}

void Level::tickTiles()
{
	activeChunks.clear();
	for (const auto &player : players)
	{
		int_t chunkX = Mth::floor(player->x / 16.0);
		int_t chunkZ = Mth::floor(player->z / 16.0);
		for (int_t dx = -9; dx <= 9; ++dx)
			for (int_t dz = -9; dz <= 9; ++dz)
				activeChunks.emplaceChunk(chunkX + dx, chunkZ + dz);
	}
	if (delayUntilNextMoodSound > 0)
		--delayUntilNextMoodSound;

	auto nextPosition = [&]() {
		uint_t value = static_cast<uint_t>(randValue) * 3U + static_cast<uint_t>(addend);
		std::memcpy(&randValue, &value, sizeof(value));
		return value >> 2;
	};
	for (const auto &position : activeChunks)
	{
		int_t baseX = position.x * 16;
		int_t baseZ = position.z * 16;
		auto chunk = getChunk(position.x, position.z);
		if (delayUntilNextMoodSound == 0)
		{
			uint_t value = nextPosition();
			int_t x = value & 15;
			int_t z = (value >> 8) & 15;
			int_t y = (value >> 16) & 127;
			int_t tile = chunk->getTile(x, y, z);
			x += baseX;
			z += baseZ;
			if (tile == 0 && getFullBrightness(x, y, z) <= random.nextInt(8) &&
				getBrightness(LightLayer::Sky, x, y, z) <= 0)
			{
				auto player = getNearestPlayer(x + 0.5, y + 0.5, z + 0.5, 8.0);
				if (player != nullptr && player->distanceToSqr(x + 0.5, y + 0.5, z + 0.5) > 4.0)
				{
					playSoundEffect(x + 0.5, y + 0.5, z + 0.5, u"ambient.cave.cave", 0.7f, 0.8f + random.nextFloat() * 0.2f);
					delayUntilNextMoodSound = random.nextInt(12000) + 6000;
				}
			}
		}

		if (random.nextInt(100000) == 0 && isRaining() && isThundering())
		{
			uint_t value = nextPosition();
			int_t x = baseX + (value & 15);
			int_t z = baseZ + ((value >> 8) & 15);
			int_t y = findTopSolidBlock(x, z);
			if (canBlockBeRainedOn(x, y, z))
			{
				addWeatherEffect(std::make_shared<EntityLightningBolt>(*this, x, y, z));
				lastLightningBolt = 2;
			}
		}

		if (random.nextInt(16) == 0)
		{
			uint_t value = nextPosition();
			int_t x = value & 15;
			int_t z = (value >> 8) & 15;
			int_t y = findTopSolidBlock(x + baseX, z + baseZ);
			if (getBiomeSource().getBiomeInfo(getBiomeSource().getBiome(x + baseX, z + baseZ)).enableSnow &&
				y >= 0 && y < DEPTH && chunk->getBrightness(LightLayer::Block, x, y, z) < 10)
			{
				int_t below = chunk->getTile(x, y - 1, z);
				int_t at = chunk->getTile(x, y, z);
				if (isRaining() && at == 0 && Tile::snow.mayPlace(*this, x + baseX, y, z + baseZ) &&
					below != 0 && below != Tile::ice.id && Tile::tiles[below]->material.isSolid())
					setTile(x + baseX, y, z + baseZ, Tile::snow.id);
				if (below == Tile::calmWater.id && chunk->getData(x, y - 1, z) == 0)
					setTile(x + baseX, y - 1, z + baseZ, Tile::ice.id);
			}
		}

		for (int_t i = 0; i < 80; ++i)
		{
			uint_t value = nextPosition();
			int_t x = value & 15;
			int_t z = (value >> 8) & 15;
			int_t y = (value >> 16) & 127;
			int_t tile = chunk->getTile(x, y, z);
			if (Tile::shouldTick[tile])
				Tile::tiles[tile]->tick(*this, x + baseX, y, z + baseZ, random);
		}
	}
}

bool Level::tickPendingTicks(bool unknown)
{
	int_t size = static_cast<int_t>(tickNextTickList.size());
	if (size != static_cast<int_t>(tickNextTickSet.size()))
		throw std::runtime_error("TickNextTick list out of synch");

	if (size > MAX_TICK_TILES_PER_TICK)
		size = MAX_TICK_TILES_PER_TICK;

	for (int_t i = 0; i < size; i++)
	{
		auto it = tickNextTickList.begin();
		if (it == tickNextTickList.end())
			break;

		TickNextTickData entry = *it;
		if (!unknown && entry.delay > time)
			break;

		tickNextTickList.erase(it);
		tickNextTickSet.erase(entry);

		constexpr int_t chunkRadius = 8;
		if (!hasChunksAt(entry.x - chunkRadius, entry.y - chunkRadius, entry.z - chunkRadius, entry.x + chunkRadius, entry.y + chunkRadius, entry.z + chunkRadius))
			continue;

		int_t tile = getTile(entry.x, entry.y, entry.z);
		if (tile == entry.tileId && tile > 0 && Tile::tiles[tile] != nullptr)
			Tile::tiles[tile]->tick(*this, entry.x, entry.y, entry.z, random);
	}

	return !tickNextTickList.empty();
}

void Level::animateTick(int_t x, int_t y, int_t z)
{
	constexpr int_t range = 16;
	Random tickRandom;

	for (int_t i = 0; i < 1000; i++)
	{
		int_t xt = x + random.nextInt(range) - random.nextInt(range);
		int_t yt = y + random.nextInt(range) - random.nextInt(range);
		int_t zt = z + random.nextInt(range) - random.nextInt(range);

		if (!hasChunkAt(xt, yt, zt))
			continue;

		int_t tile = getTile(xt, yt, zt);
		if (tile > 0 && Tile::tiles[tile] != nullptr)
			Tile::tiles[tile]->animateTick(*this, xt, yt, zt, tickRandom);
	}
}

const std::vector<std::shared_ptr<Entity>> &Level::getEntities(Entity *ignore, AABB &aabb)
{
	es.clear();

	int_t x0 = Mth::floor((aabb.x0 - 2.0) / 16.0);
	int_t x1 = Mth::floor((aabb.x1 + 2.0) / 16.0);
	int_t z0 = Mth::floor((aabb.z0 - 2.0) / 16.0);
	int_t z1 = Mth::floor((aabb.z1 + 2.0) / 16.0);

	for (int_t cx = x0; cx <= x1; cx++)
		for (int_t cz = z0; cz <= z1; cz++)
			if (hasChunk(cx, cz))
				getChunk(cx, cz)->getEntities(ignore, aabb, es);

	return es;
}

std::vector<std::shared_ptr<Entity>> Level::getEntitiesOfCondition(bool (*condition)(Entity &), AABB &aabb)
{
	std::vector<std::shared_ptr<Entity>> result;

	int_t x0 = Mth::floor((aabb.x0 - 2.0) / 16.0);
	int_t x1 = Mth::floor((aabb.x1 + 2.0) / 16.0);
	int_t z0 = Mth::floor((aabb.z0 - 2.0) / 16.0);
	int_t z1 = Mth::floor((aabb.z1 + 2.0) / 16.0);

	for (int_t cx = x0; cx <= x1; cx++)
		for (int_t cz = z0; cz <= z1; cz++)
			if (hasChunk(cx, cz))
				getChunk(cx, cz)->getEntitiesOfCondition(condition, aabb, result);

	return result;
}

const std::vector<std::shared_ptr<Entity>> &Level::getAllEntities()
{
	return entities;
}

void Level::tileEntityChanged(int_t x, int_t y, int_t z, std::shared_ptr<TileEntity> tileEntity)
{
	if (hasChunkAt(x, y, z))
		getChunkAt(x, z)->markUnsaved();
	for (size_t i = 0; i < listeners.size(); ++i)
		listeners[i]->tileEntityChanged(x, y, z, tileEntity);
}

int_t Level::countConditionOf(bool (*condition)(Entity &))
{
	int_t count = 0;
	for (size_t i = 0; i < entities.size(); ++i)
	{
		auto entity = entities[i];
		if (condition(*entity))
			++count;
	}
	return count;
}

void Level::addEntities(const std::vector<std::shared_ptr<Entity>> &entities)
{
	this->entities.insert(this->entities.end(), entities.begin(), entities.end());
	for (size_t i = 0; i < entities.size(); ++i)
		entityAdded(entities[i]);
}

void Level::removeEntities(const std::vector<std::shared_ptr<Entity>> &entities)
{
	entitiesToRemove.insert(entitiesToRemove.end(), entities.begin(), entities.end());
}

void Level::disconnect()
{

}

void Level::checkSession()
{
	std::unique_ptr<File> session_lock(File::open(*dir, u"session.lock"));
	std::unique_ptr<std::istream> is(session_lock->toStreamIn());
	if (!is)
		throw MinecraftException("Failed to check session lock, aborting");

	if (IOUtil::readLong(*is) != sessionId)
		throw MinecraftException("The save is being accessed from another location, aborting");
}

void Level::setTime(long_t time)
{
	this->time = time;
}

void Level::ensureAdded(std::shared_ptr<Entity> entity)
{
	int_t x = Mth::floor(entity->x / 16.0);
	int_t z = Mth::floor(entity->z / 16.0);
	byte_t rad = 2;

	for (int_t cx = x - rad; cx <= x + rad; cx++)
		for (int_t cz = z - rad; cz <= z + rad; cz++)
			getChunk(cx, cz);

	for (auto &i : entities)
		if (i->entityId == entity->entityId)
			return;
	entities.push_back(entity);
}

bool Level::mayInteract(std::shared_ptr<Player> player, int_t x, int_t y, int_t z)
{
	return true;
}

void Level::broadcastEntityEvent(std::shared_ptr<Entity> entity, byte_t event)
{

}

std::shared_ptr<ChunkSource> Level::getChunkSource()
{
	return chunkSource;
}

bool Level::isRaining()
{
	return getRainStrength(1.0f) > 0.2f;
}


bool Level::isThundering()
{
	return getThunderStrength(1.0f) > 0.9f;
}


void Level::updateWeather()
{
	if (dimension != nullptr && dimension->foggy)
		return;

	if (lastLightningBolt > 0)
		lastLightningBolt--;

	if (thunderTime <= 0)
	{
		if (thundering)
			thunderTime = random.nextInt(12000) + 3600;
		else
			thunderTime = random.nextInt(168000) + 12000;
	}
	else
	{
		thunderTime--;
		if (thunderTime <= 0)
			thundering = !thundering;
	}

	if (rainTime <= 0)
	{
		if (raining)
			rainTime = random.nextInt(12000) + 12000;
		else
			rainTime = random.nextInt(168000) + 12000;
	}
	else
	{
		rainTime--;
		if (rainTime <= 0)
			raining = !raining;
	}

	previousRainingStrength = rainingStrength;
	if (raining)
		rainingStrength = static_cast<float>(rainingStrength + 0.01);
	else
		rainingStrength = static_cast<float>(rainingStrength - 0.01);

	if (rainingStrength < 0.0f)
		rainingStrength = 0.0f;
	if (rainingStrength > 1.0f)
		rainingStrength = 1.0f;

	previousThunderingStrength = thunderingStrength;
	if (thundering)
		thunderingStrength = static_cast<float>(thunderingStrength + 0.01);
	else
		thunderingStrength = static_cast<float>(thunderingStrength - 0.01);

	if (thunderingStrength < 0.0f)
		thunderingStrength = 0.0f;
	if (thunderingStrength > 1.0f)
		thunderingStrength = 1.0f;
}


void Level::stopPrecipitation()
{
	rainTime = 0;
	raining = false;
	thunderTime = 0;
	thundering = false;
}



void Level::setWeather(bool rain, bool thunder)
{
	raining = rain;
	rainTime = rain ? random.nextInt(12000) + 12000 : 0;
	thundering = thunder;
	thunderTime = thunder ? random.nextInt(12000) + 3600 : 0;
}


float Level::getRainStrength(float a) const
{
	return previousRainingStrength + (rainingStrength - previousRainingStrength) * a;
}


float Level::getThunderStrength(float a) const
{
	return (previousThunderingStrength + (thunderingStrength - previousThunderingStrength) * a) * getRainStrength(a);
}


void Level::setRainStrength(float strength)
{
	previousRainingStrength = strength;
	rainingStrength = strength;
}

bool Level::canBlockBeRainedOn(int_t x, int_t y, int_t z)
{
	if (!isRaining())
		return false;
	if (!canSeeSky(x, y, z))
		return false;
	int_t top = -1;
	for (int_t iy = 127; iy > 0; iy--)
	{
		const Material &material = getMaterial(x, iy, z);
		if (material.isSolid() || material.isLiquid())
		{
			top = iy + 1;
			break;
		}
	}
	if (top > y)
		return false;
	const BiomeInfo &biome = getBiomeSource().getBiomeInfo(getBiomeSource().getBiome(x, z));
	return !biome.enableSnow && biome.enableRain;
}

MapDataBase *Level::loadItemData(const jstring &id, const std::function<std::unique_ptr<MapDataBase>(const jstring&)> &factory)
{
	auto it = loadedItemData->find(id);
	if (it != loadedItemData->end())
		return it->second.get();

	if (dir != nullptr)
	{
		std::unique_ptr<File> dataDir(File::open(*dir, u"data"));
		if (dataDir != nullptr)
		{
			std::unique_ptr<File> file(File::open(*dataDir, id + u".dat"));
			if (file != nullptr && file->exists())
			{
				try
				{
					std::unique_ptr<std::istream> is(file->toStreamIn());
					if (is)
					{
						std::unique_ptr<CompoundTag> root(NbtIo::readCompressed(*is));
						if (root != nullptr && root->contains(u"data"))
						{
							auto dataTag = root->getCompound(u"data");
							auto data = factory(id);
							if (data != nullptr)
							{
								data->readFromNBT(*dataTag);
								(*loadedItemData)[id] = std::move(data);
							}
						}
					}
				}
				catch (...) {}
			}
		}
	}

	auto it2 = loadedItemData->find(id);
	if (it2 != loadedItemData->end())
		return it2->second.get();
	return nullptr;
}

void Level::setItemData(const jstring &id, MapDataBase *data)
{
	if (data == nullptr)
		return;
	(*loadedItemData)[id] = std::unique_ptr<MapDataBase>(data);
}

void Level::shareItemDataWith(Level &level)
{
	loadedItemData = level.loadedItemData;
	idCounts = level.idCounts;
}

int_t Level::getUniqueDataId(const jstring &key)
{
	short_t value = 0;
	auto it = idCounts->find(key);
	if (it != idCounts->end())
		value = it->second + 1;
	(*idCounts)[key] = value;

	if (dir != nullptr)
	{
		try
		{
			std::unique_ptr<File> dataDir(File::open(*dir, u"data"));
			if (dataDir != nullptr)
			{
				if (!dataDir->exists())
					dataDir->mkdirs();
				std::unique_ptr<File> idFile(File::open(*dataDir, u"idcounts.dat"));
				if (idFile != nullptr)
				{
					auto root = Util::make_shared<CompoundTag>();
					for (const auto &entry : *idCounts)
						root->putShort(entry.first, entry.second);
					std::unique_ptr<std::ostream> os(idFile->toStreamOut());
					if (os)
						NbtIo::writeCompressed(*root, *os);
				}
			}
		}
		catch (...) {}
	}
	return value;
}

void Level::saveAllItemData()
{
	if (dir == nullptr)
		return;

	std::unique_ptr<File> dataDir(File::open(*dir, u"data"));
	if (dataDir == nullptr)
		return;
	if (!dataDir->exists())
		dataDir->mkdirs();

	for (auto &entry : *loadedItemData)
	{
		MapDataBase *data = entry.second.get();
		if (data == nullptr || !data->isDirty())
			continue;

		try
			{
			std::unique_ptr<File> file(File::open(*dataDir, entry.first + u".dat"));
			if (file == nullptr)
				continue;
			auto root = Util::make_shared<CompoundTag>();
			auto dataTag = Util::make_shared<CompoundTag>();
			data->writeToNBT(*dataTag);
			root->put(u"data", dataTag);
			std::unique_ptr<std::ostream> os(file->toStreamOut());
			if (os)
			{
				NbtIo::writeCompressed(*root, *os);
				data->setDirty(false);
			}
		}
		catch (...) {}
	}
}
