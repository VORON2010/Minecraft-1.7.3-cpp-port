#include "world/level/chunk/LevelChunk.h"

#include <algorithm>
#include <cstring>

#include "world/level/Level.h"
#include "world/level/tile/Tile.h"

#include "util/Mth.h"

bool LevelChunk::touchedSky = false;
namespace
{
	std::shared_ptr<Level> makeLevelHandle(Level &level)
	{
		return std::shared_ptr<Level>(&level, [](Level *) {});
	}
}


LevelChunk::LevelChunk(Level &level, int_t x, int_t z) : level(level)
{
	this->x = x;
	this->z = z;
}

LevelChunk::LevelChunk(Level &level, const ubyte_t *blocks, int_t x, int_t z) : LevelChunk(level, x, z)
{
	std::copy(blocks, blocks + this->blocks.size(), this->blocks.begin());
}

bool LevelChunk::isAt(int_t x, int_t z)
{
	return (this->x == x) && (this->z == z);
}

int_t LevelChunk::getHeightmap(int_t x, int_t z)
{
	return heightmap[(z * 16) | x];
}

void LevelChunk::recalcBlockLights()
{

}

void LevelChunk::recalcHeightmapOnly()
{
	int_t min = Level::DEPTH - 1;

	for (int_t x = 0; x < 16; x++)
	{
		for (int_t z = 0; z < 16; z++)
		{
			int_t y = Level::DEPTH - 1;
			int_t i = ((x * 16) + z) * Level::DEPTH;
			while (y > 0 && Tile::lightBlock[blocks[i + y - 1]] == 0)
				y--;
			heightmap[(z * 16) | x] = y;
			if (y < min)
				min = y;
		}
	}

	minHeight = min;
	unsaved = true;
}

void LevelChunk::recalcHeightmap()
{
	int_t min = Level::DEPTH - 1;

	for (int_t x = 0; x < 16; x++)
	{
		for (int_t z = 0; z < 16; z++)
		{
			
			int_t y = Level::DEPTH - 1;
			int_t i = ((x * 16) + z) * Level::DEPTH;
			while (y > 0 && Tile::lightBlock[blocks[i + y - 1]] == 0)
				y--;
			heightmap[(z * 16) | x] = y;
			if (y < min)
				min = y;

			
			if (!level.dimension->hasCeiling)
			{
				int_t light = 15;
				int_t ly = Level::DEPTH - 1;
				do
				{
					light -= Tile::lightBlock[static_cast<ubyte_t>(blocks[i + ly])];
					if (light > 0)
						skyLight.set(x, ly, z, light);
				} while (--ly > 0 && light > 0);
			}
		}
	}

	minHeight = min;

	for (int_t x = 0; x < 16; x++)
		for (int_t z = 0; z < 16; z++)
			lightGaps(x, z);

	unsaved = true;
}

void LevelChunk::lightLava()
{
	
}

void LevelChunk::lightGaps(int_t x, int_t z)
{
	int_t height = getHeightmap(x, z);
	int_t wx = this->x * 16 + x;
	int_t wz = this->z * 16 + z;
	lightGap(wx - 1, wz, height);
	lightGap(wx + 1, wz, height);
	lightGap(wx, wz - 1, height);
	lightGap(wx, wz + 1, height);
}

void LevelChunk::lightGap(int_t x, int_t z, int_t otherHeight)
{
	int_t levelHeight = level.getHeightmap(x, z);
	if (levelHeight > otherHeight)
	{
		level.updateLight(LightLayer::Sky, x, otherHeight, z, x, levelHeight, z);
		unsaved = true;
	}
	else if (levelHeight < otherHeight)
	{
		level.updateLight(LightLayer::Sky, x, levelHeight, z, x, otherHeight, z);
		unsaved = true;
	}
}

void LevelChunk::recalcHeight(int_t x, int_t y, int_t z)
{
	int_t oldHeight = heightmap[(z * 16) | x];
	int_t newHeight = oldHeight;
	if (y > oldHeight)
		newHeight = y;

	int_t ri = (x * (16 * 128)) | (z * 128);
	while (newHeight > 0 && Tile::lightBlock[blocks[ri + newHeight - 1]] == 0)
		newHeight--;

	if (newHeight == oldHeight)
		return;

	level.lightColumnChanged(x, z, newHeight, oldHeight);
	heightmap[(z * 16) | x] = newHeight;

	if (newHeight < minHeight)
	{
		minHeight = newHeight;
	}
	else
	{
		int_t iy = Level::DEPTH - 1;
		for (int_t x = 0; x < 16; x++)
			for (int_t z = 0; z < 16; z++)
				if (heightmap[(z * 16) | x] < iy)
					iy = heightmap[(z * 16) | x];
		minHeight = iy;
	}

	int_t wx = this->x * 16 + x;
	int_t wz = this->z * 16 + z;
	if (newHeight < oldHeight)
	{
		for (int_t yi = newHeight; yi < oldHeight; yi++)
			skyLight.set(x, yi, z, 15);
	}
	else
	{
		level.updateLight(LightLayer::Sky, wx, oldHeight, wz, wx, newHeight, wz);
		for (int_t yi = oldHeight; yi < newHeight; yi++)
			skyLight.set(x, yi, z, 0);
	}

	int_t light = 15;
	int_t ly = newHeight;
	while (newHeight > 0 && light > 0)
	{
		--newHeight;

		int_t block = Tile::lightBlock[getTile(x, newHeight, z)];
		if (block == 0)
			block = 1;
		light -= block;
		if (light <= 0)
			light = 0;
		skyLight.set(x, newHeight, z, light);
	}

	while (newHeight > 0 && Tile::lightBlock[getTile(x, newHeight - 1, z)] == 0)
		newHeight--;
	if (newHeight != ly)
		level.updateLight(LightLayer::Sky, wx - 1, newHeight, wz - 1, wx + 1, ly, wz + 1);

	unsaved = true;
}

int_t LevelChunk::getTile(int_t x, int_t y, int_t z)
{
	return blocks[(x * (16 * 128)) | (z * 128) | y];
}

bool LevelChunk::setTileAndData(int_t x, int_t y, int_t z, int_t tile, int_t data)
{
	int_t oldHeight = heightmap[(z * 16) | x];
	int_t index = (x * 128 * 16) | (z * 128) | y;
	int_t oldTile = blocks[index];
	int_t oldData = this->data.get(x, y, z);
	if (oldTile == tile && oldData == data)
		return false;

	int_t wx = this->x * 16 + x;
	int_t wz = this->z * 16 + z;

	
	
	
	blocks[index] = tile;
	if (oldTile != 0 && !level.isOnline)
		Tile::tiles[oldTile]->onRemove(level, wx, y, wz);

	this->data.set(x, y, z, data);

	if (!level.dimension->hasCeiling)
	{
		if (Tile::lightBlock[static_cast<ubyte_t>(tile)] != 0)
		{
			if (y >= oldHeight)
				recalcHeight(x, y + 1, z);
		}
		else if (y == oldHeight - 1)
		{
			recalcHeight(x, y, z);
		}
		level.updateLight(LightLayer::Sky, wx, y, wz, wx, y, wz);
	}
	level.updateLight(LightLayer::Block, wx, y, wz, wx, y, wz);
	lightGaps(x, z);
	this->data.set(x, y, z, data);
	if (tile != 0)
		Tile::tiles[tile]->onPlace(level, wx, y, wz);

	unsaved = true;
	return true;
}

bool LevelChunk::setTile(int_t x, int_t y, int_t z, int_t tile)
{
	int_t oldHeight = heightmap[(z * 16) | x];
	int_t oldTile = blocks[(x * (16 * 128)) | (z * 128) | y];
	if (oldTile == tile)
		return false;

	int_t wx = this->x * 16 + x;
	int_t wz = this->z * 16 + z;
	blocks[(x * 128 * 16) | (z * 128) | y] = tile;
	if (oldTile != 0)
		Tile::tiles[oldTile]->onRemove(level, wx, y, wz);
	data.set(x, y, z, 0);

	if (Tile::lightBlock[static_cast<ubyte_t>(tile)] != 0)
	{
		if (y >= oldHeight)
			recalcHeight(x, y + 1, z);
	}
	else if (y == oldHeight - 1)
	{
		recalcHeight(x, y, z);
	}

	level.updateLight(LightLayer::Sky, wx, y, wz, wx, y, wz);
	level.updateLight(LightLayer::Block, wx, y, wz, wx, y, wz);
	lightGaps(x, z);
	if (tile != 0 && !level.isOnline)
		Tile::tiles[tile]->onPlace(level, wx, y, wz);

	unsaved = true;
	return true;
}

int_t LevelChunk::getData(int_t x, int_t y, int_t z)
{
	return data.get(x, y, z);
}

void LevelChunk::setData(int_t x, int_t y, int_t z, int_t data)
{
	unsaved = true;
	this->data.set(x, y, z, data);
}

int_t LevelChunk::getBrightness(int_t lightLayer, int_t x, int_t y, int_t z)
{
	if (lightLayer == LightLayer::Sky)
		return skyLight.get(x, y, z);
	if (lightLayer == LightLayer::Block)
		return blockLight.get(x, y, z);
	return 0;
}

void LevelChunk::setBrightness(int_t lightLayer, int_t x, int_t y, int_t z, int_t brightness)
{
	unsaved = true;
	if (lightLayer == LightLayer::Sky)
		skyLight.set(x, y, z, brightness);
	else if (lightLayer == LightLayer::Block)
		blockLight.set(x, y, z, brightness);
}

int_t LevelChunk::getRawBrightness(int_t x, int_t y, int_t z, int_t darken)
{
	int_t light = skyLight.get(x, y, z);
	if (light > 0)
		touchedSky = true;
	light -= darken;

	int_t lightBlock = blockLight.get(x, y, z);
	if (lightBlock > light)
		light = lightBlock;

	return light;
}

void LevelChunk::addEntity(std::shared_ptr<Entity> entity)
{
	lastSaveHadEntities = true;

	int_t y = Mth::floor(entity->y / 16.0);
	if (y < 0)
		y = 0;
	if (y >= static_cast<int_t>(entityBlocks.size()))
		y = static_cast<int_t>(entityBlocks.size()) - 1;

	entity->inChunk = true;
	entity->xChunk = x;
	entity->yChunk = y;
	entity->zChunk = z;
	entityBlocks[y].push_back(entity);
}

void LevelChunk::removeEntity(std::shared_ptr<Entity> entity)
{
	removeEntity(entity, entity->yChunk);
}

void LevelChunk::removeEntity(std::shared_ptr<Entity> entity, int_t y)
{
	if (y < 0)
		y = 0;
	if (y >= static_cast<int_t>(entityBlocks.size()))
		y = static_cast<int_t>(entityBlocks.size()) - 1;
	auto &entities = entityBlocks[y];
	auto found = std::find_if(entities.begin(), entities.end(), [&](const auto &candidate) {
		return candidate->entityId == entity->entityId;
	});
	if (found != entities.end())
		entities.erase(found);
}

bool LevelChunk::isSkyLit(int_t x, int_t y, int_t z)
{
	return y >= heightmap[(z * 16) | x];
}

void LevelChunk::skyBrightnessChanged()
{
	int_t x0 = x * 16;
	int_t y0 = minHeight - 16;
	int_t z0 = z * 16;
	int_t x1 = x * 16 + 16;
	int_t y1 = Level::DEPTH;
	int_t z1 = z * 16 + 16;
	level.setTilesDirty(x0, y0, z0, x1, y1, z1);
}

std::shared_ptr<TileEntity> LevelChunk::getTileEntity(int_t x, int_t y, int_t z)
{
	TilePos position(x, y, z);
	auto found = tileEntities.find(position);
	if (found == tileEntities.end())
	{
		int_t tile = getTile(x, y, z);
		if (!Tile::isEntityTile[tile])
			return nullptr;
		Tile::tiles[tile]->onPlace(level, this->x * 16 + x, y, this->z * 16 + z);
		found = tileEntities.find(position);
	}
	if (found == tileEntities.end())
		return nullptr;
	auto tileEntity = found->second;
	if (tileEntity->isRemoved())
	{
		tileEntities.erase(found);
		tileEntityPositions.erase(position);
		return nullptr;
	}
	return tileEntity;
}

void LevelChunk::addTileEntity(std::shared_ptr<TileEntity> tileEntity)
{
	setTileEntity(tileEntity->x - x * 16, tileEntity->y, tileEntity->z - z * 16, tileEntity);
	if (loaded)
		level.tileEntityList.push_back(tileEntity);
}

void LevelChunk::setTileEntity(int_t x, int_t y, int_t z, std::shared_ptr<TileEntity> tileEntity)
{
	tileEntity->level = makeLevelHandle(level);
	tileEntity->x = this->x * 16 + x;
	tileEntity->y = y;
	tileEntity->z = this->z * 16 + z;
	int_t tile = getTile(x, y, z);
	if (tile != 0 && Tile::isEntityTile[tile])
	{
		tileEntity->clearRemoved();
		tileEntities[TilePos(x, y, z)] = tileEntity;
		tileEntityPositions.emplace(x, y, z);
	}
	else
		std::cout << "Attempted to place a tile entity where there was no entity tile!\n";
}

void LevelChunk::removeTileEntity(int_t x, int_t y, int_t z)
{
	if (!loaded)
		return;
	TilePos position(x, y, z);
	auto found = tileEntities.find(position);
	if (found != tileEntities.end())
	{
		found->second->setRemoved();
		tileEntities.erase(found);
		tileEntityPositions.erase(position);
	}
}

void LevelChunk::load()
{
	loaded = true;
	std::vector<std::shared_ptr<TileEntity>> tileEntitiesToAdd;
	tileEntitiesToAdd.reserve(tileEntities.size());
	for (const auto &position : tileEntityPositions)
		tileEntitiesToAdd.push_back(tileEntities.at(position));
	level.addTileEntities(tileEntitiesToAdd);
	for (const auto &entities : entityBlocks)
		level.addEntities(entities);
}

void LevelChunk::unload()
{
	loaded = false;
	for (const auto &position : tileEntityPositions)
		tileEntities.at(position)->setRemoved();
	for (const auto &entities : entityBlocks)
		level.removeEntities(entities);
}

void LevelChunk::markUnsaved()
{
	unsaved = true;
}

void LevelChunk::getEntities(Entity *ignore, AABB &aabb, std::vector<std::shared_ptr<Entity>> &entities)
{
	int_t y0 = Mth::floor((aabb.y0 - 2.0) / 16.0);
	int_t y1 = Mth::floor((aabb.y1 + 2.0) / 16.0);
	if (y0 < 0)
		y0 = 0;
	if (y1 >= static_cast<int_t>(entityBlocks.size()))
		y1 = static_cast<int_t>(entityBlocks.size()) - 1;

	for (int_t y = y0; y <= y1; y++)
	{
		for (const auto &entity : entityBlocks[y])
		{
			if (entity.get() != ignore && entity->bb.intersects(aabb))
				entities.push_back(entity);
		}
	}
}

void LevelChunk::getEntitiesOfCondition(bool (*condition)(Entity &), AABB &aabb, std::vector<std::shared_ptr<Entity>> &entities)
{
	int_t y0 = Mth::floor((aabb.y0 - 2.0) / 16.0);
	int_t y1 = Mth::floor((aabb.y1 + 2.0) / 16.0);
	if (y0 < 0)
		y0 = 0;
	if (y1 >= static_cast<int_t>(entityBlocks.size()))
		y1 = static_cast<int_t>(entityBlocks.size()) - 1;

	for (int_t y = y0; y <= y1; y++)
	{
		for (size_t i = 0; i < entityBlocks[y].size(); ++i)
		{
			auto entity = entityBlocks[y][i];
			if (condition(*entity) && entity->bb.intersects(aabb))
				entities.push_back(entity);
		}
	}
}

int_t LevelChunk::countEntities()
{
	int_t count = 0;
	for (auto &b : entityBlocks)
		count += static_cast<int_t>(b.size());
	return count;
}

bool LevelChunk::shouldSave(bool force)
{
	if (dontSave)
		return false;

	if (force)
	{
		if (lastSaveHadEntities && level.time != lastSaveTime)
			return true;
	}
	else
	{
		if (lastSaveHadEntities && level.time >= lastSaveTime + 600)
			return true;
	}

	return unsaved;
}

void LevelChunk::setBlocks(byte_t *blocks, int_t y)
{

}

int_t LevelChunk::getBlocksAndData(byte_t *out, int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1)
{
	return 0;
}

int_t LevelChunk::setBlocksAndData(const byte_t *in, int_t x0, int_t y0, int_t z0, int_t x1, int_t y1, int_t z1)
{
	int_t offset = 0;

	for (int_t x = x0; x < x1; x++)
	{
		for (int_t z = z0; z < z1; z++)
		{
			int_t index = (x << 11) | (z << 7) | y0;
			int_t length = y1 - y0;
			std::memcpy(blocks.data() + index, in + offset, length);
			offset += length;
		}
	}

	recalcHeightmapOnly();

	for (int_t x = x0; x < x1; x++)
	{
		for (int_t z = z0; z < z1; z++)
		{
			int_t index = ((x << 11) | (z << 7) | y0) >> 1;
			int_t length = (y1 - y0) / 2;
			std::memcpy(data.data.data() + index, in + offset, length);
			offset += length;
		}
	}

	for (int_t x = x0; x < x1; x++)
	{
		for (int_t z = z0; z < z1; z++)
		{
			int_t index = ((x << 11) | (z << 7) | y0) >> 1;
			int_t length = (y1 - y0) / 2;
			std::memcpy(blockLight.data.data() + index, in + offset, length);
			offset += length;
		}
	}

	for (int_t x = x0; x < x1; x++)
	{
		for (int_t z = z0; z < z1; z++)
		{
			int_t index = ((x << 11) | (z << 7) | y0) >> 1;
			int_t length = (y1 - y0) / 2;
			std::memcpy(skyLight.data.data() + index, in + offset, length);
			offset += length;
		}
	}

	return offset;
}

Random LevelChunk::getRandom(long_t seed)
{
	auto widen = [](uint_t value) {
		int_t signedValue;
		std::memcpy(&signedValue, &value, sizeof(value));
		return static_cast<ulong_t>(static_cast<long_t>(signedValue));
	};
	uint_t ux = static_cast<uint_t>(x);
	uint_t uz = static_cast<uint_t>(z);
	ulong_t mixed = static_cast<ulong_t>(level.seed) + widen(ux * ux * 4987142U) +
		widen(ux * 5947611U) + widen(uz * uz) * 4392871ULL + widen(uz * 389711U);
	mixed ^= static_cast<ulong_t>(seed);
	long_t signedSeed;
	std::memcpy(&signedSeed, &mixed, sizeof(mixed));
	return Random(signedSeed);
}

bool LevelChunk::isEmpty()
{
	return false;
}
