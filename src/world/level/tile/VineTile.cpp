#include "world/level/tile/VineTile.h"

#include <algorithm>
#include <memory>

#include "world/entity/item/EntityItem.h"
#include "world/entity/player/Player.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/level/FoliageColor.h"
#include "world/level/Level.h"
#include "world/level/LevelSource.h"
#include "world/stats/StatList.h"
#include "world/level/biome/Biome.h"
#include "world/level/biome/BiomeSource.h"

namespace
{
	// horizontal directions 0..3 (south, west, north, east), same order as the data bits
	constexpr int_t DIR_X[4] = {0, -1, 0, 1};
	constexpr int_t DIR_Z[4] = {1, 0, -1, 0};
	// face -> direction
	constexpr int_t FACE_TO_DIR[6] = {-1, -1, 2, 0, 1, 3};
}

VineTile::VineTile(int_t id) : Tile(id, 143, Material::plants())
{
	setTicking(true);
	updateCachedProperties();
}

bool VineTile::isCubeShaped()
{
	return false;
}

bool VineTile::isSolidRender()
{
	return false;
}

void VineTile::updateDefaultShape()
{
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

void VineTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	int_t data = level.getData(x, y, z);
	float x0 = 1.0f, y0 = 1.0f, z0 = 1.0f;
	float x1 = 0.0f, y1 = 0.0f, z1 = 0.0f;
	bool attached = data > 0;

	if ((data & 2) != 0)
	{
		x1 = std::max(x1, 0.0625f);
		x0 = 0.0f; y0 = 0.0f; y1 = 1.0f; z0 = 0.0f; z1 = 1.0f;
		attached = true;
	}
	if ((data & 8) != 0)
	{
		x0 = std::min(x0, 0.9375f);
		x1 = 1.0f; y0 = 0.0f; y1 = 1.0f; z0 = 0.0f; z1 = 1.0f;
		attached = true;
	}
	if ((data & 4) != 0)
	{
		z1 = std::max(z1, 0.0625f);
		z0 = 0.0f; x0 = 0.0f; x1 = 1.0f; y0 = 0.0f; y1 = 1.0f;
		attached = true;
	}
	if ((data & 1) != 0)
	{
		z0 = std::min(z0, 0.9375f);
		z1 = 1.0f; x0 = 0.0f; x1 = 1.0f; y0 = 0.0f; y1 = 1.0f;
		attached = true;
	}
	if (!attached && canGrowOn(level.getTile(x, y + 1, z)))
	{
		y0 = std::min(y0, 0.9375f);
		y1 = 1.0f; x0 = 0.0f; x1 = 1.0f; z0 = 0.0f; z1 = 1.0f;
	}

	setShape(x0, y0, z0, x1, y1, z1);
}

AABB *VineTile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	(void)level; (void)x; (void)y; (void)z;
	return nullptr;
}

bool VineTile::canGrowOn(int_t tile)
{
	if (tile == 0)
		return false;
	Tile *t = Tile::tiles[tile];
	return t != nullptr && t->isCubeShaped() && t->material.blocksMotion();
}

bool VineTile::mayPlaceOnFace(Level &level, int_t x, int_t y, int_t z, Facing face)
{
	switch (face)
	{
	case Facing::UP: return canGrowOn(level.getTile(x, y + 1, z));
	case Facing::NORTH: return canGrowOn(level.getTile(x, y, z + 1));
	case Facing::SOUTH: return canGrowOn(level.getTile(x, y, z - 1));
	case Facing::WEST: return canGrowOn(level.getTile(x + 1, y, z));
	case Facing::EAST: return canGrowOn(level.getTile(x - 1, y, z));
	default: return false;
	}
}

bool VineTile::updateAttachments(Level &level, int_t x, int_t y, int_t z)
{
	int_t data = level.getData(x, y, z);
	int_t newData = data;
	if (data > 0)
	{
		for (int_t dir = 0; dir <= 3; ++dir)
		{
			int_t bit = 1 << dir;
			if ((data & bit) != 0 && !canGrowOn(level.getTile(x + DIR_X[dir], y, z + DIR_Z[dir])) &&
				(level.getTile(x, y + 1, z) != id || (level.getData(x, y + 1, z) & bit) == 0))
				newData &= ~bit;
		}
	}

	if (newData == 0 && !canGrowOn(level.getTile(x, y + 1, z)))
		return false;
	if (newData != data)
		level.setData(x, y, z, newData);
	return true;
}

int_t VineTile::getColor(LevelSource &level, int_t x, int_t y, int_t z)
{
	return level.getBiomeSource().getBiome(x, z)->getFolageColor(level, x, y, z);
}

int_t VineTile::getItemColor(int_t data)
{
	(void)data;
	return FoliageColor::getDefaultColor();
}

void VineTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	(void)tile;
	if (!level.isOnline && !updateAttachments(level, x, y, z))
	{
		spawnResources(level, x, y, z, level.getData(x, y, z));
		level.setTile(x, y, z, 0);
	}
}

void VineTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	(void)random;
	if (level.isOnline || level.random.nextInt(16) != 0)
		return;

	// too many vines around: only spread down
	int_t budget = 5;
	bool crowded = false;
	for (int_t xx = x - 4; xx <= x + 4 && !crowded; ++xx)
		for (int_t zz = z - 4; zz <= z + 4 && !crowded; ++zz)
			for (int_t yy = y - 1; yy <= y + 1; ++yy)
				if (level.getTile(xx, yy, zz) == id && --budget <= 0)
				{
					crowded = true;
					break;
				}

	int_t data = level.getData(x, y, z);
	int_t face = level.random.nextInt(6);
	int_t dir = FACE_TO_DIR[face];

	if (face == 1 && y < Level::DEPTH - 1 && level.getTile(x, y + 1, z) == 0)
	{
		if (crowded)
			return;
		int_t up = level.random.nextInt(16) & data;
		if (up > 0)
		{
			for (int_t d = 0; d <= 3; ++d)
				if (!canGrowOn(level.getTile(x + DIR_X[d], y + 1, z + DIR_Z[d])))
					up &= ~(1 << d);
			if (up > 0)
				level.setTileAndData(x, y + 1, z, id, up);
		}
	}
	else if (face >= 2 && face <= 5 && (data & (1 << dir)) == 0)
	{
		if (crowded)
			return;
		int_t nx = x + DIR_X[dir];
		int_t nz = z + DIR_Z[dir];
		int_t neighbor = level.getTile(nx, y, nz);
		if (neighbor == 0 || Tile::tiles[neighbor] == nullptr)
		{
			int_t cw = (dir + 1) & 3;
			int_t ccw = (dir + 3) & 3;
			if ((data & (1 << cw)) != 0 && canGrowOn(level.getTile(nx + DIR_X[cw], y, nz + DIR_Z[cw])))
				level.setTileAndData(nx, y, nz, id, 1 << cw);
			else if ((data & (1 << ccw)) != 0 && canGrowOn(level.getTile(nx + DIR_X[ccw], y, nz + DIR_Z[ccw])))
				level.setTileAndData(nx, y, nz, id, 1 << ccw);
			else if ((data & (1 << cw)) != 0 && level.getTile(nx + DIR_X[cw], y, nz + DIR_Z[cw]) == 0 &&
				canGrowOn(level.getTile(x + DIR_X[cw], y, z + DIR_Z[cw])))
				level.setTileAndData(nx + DIR_X[cw], y, nz + DIR_Z[cw], id, 1 << ((dir + 2) & 3));
			else if ((data & (1 << ccw)) != 0 && level.getTile(nx + DIR_X[ccw], y, nz + DIR_Z[ccw]) == 0 &&
				canGrowOn(level.getTile(x + DIR_X[ccw], y, z + DIR_Z[ccw])))
				level.setTileAndData(nx + DIR_X[ccw], y, nz + DIR_Z[ccw], id, 1 << ((dir + 2) & 3));
			else if (canGrowOn(level.getTile(nx, y + 1, nz)))
				level.setTileAndData(nx, y, nz, id, 0);
		}
		else
		{
			Tile *t = Tile::tiles[neighbor];
			if (t->material.isSolidBlocking() && t->isCubeShaped())
				level.setData(x, y, z, data | (1 << dir));
		}
	}
	else if (y > 1)
	{
		int_t below = level.getTile(x, y - 1, z);
		if (below == 0)
		{
			int_t down = level.random.nextInt(16) & data;
			if (down > 0)
				level.setTileAndData(x, y - 1, z, id, down);
		}
		else if (below == id)
		{
			int_t add = level.random.nextInt(16) & data;
			int_t belowData = level.getData(x, y - 1, z);
			if (belowData != (belowData | add))
				level.setData(x, y - 1, z, belowData | add);
		}
	}
}

void VineTile::setPlacedOnFace(Level &level, int_t x, int_t y, int_t z, Facing face)
{
	int_t data = 0;
	switch (face)
	{
	case Facing::NORTH: data = 1; break;
	case Facing::SOUTH: data = 4; break;
	case Facing::WEST: data = 8; break;
	case Facing::EAST: data = 2; break;
	default: break;
	}
	if (data != 0)
		level.setData(x, y, z, data);
}

int_t VineTile::getResource(int_t data, Random &random)
{
	(void)data;
	(void)random;
	return 0;
}

int_t VineTile::getResourceCount(Random &random)
{
	(void)random;
	return 0;
}

void VineTile::harvestBlock(Level &level, Player &player, int_t x, int_t y, int_t z, int_t data)
{
	ItemInstance *selected = player.getSelectedItem();
	if (!level.isOnline && selected != nullptr && selected->itemID == Items::shears->getShiftedIndex())
	{
		if (StatBase *stat = StatList::mineBlockStats[id])
			player.addStat(*stat, 1);
		auto item = std::make_shared<EntityItem>(level, x + 0.5, y + 0.5, z + 0.5, ItemInstance(id, 1, 0));
		item->throwTime = 10;
		level.addEntity(item);
	}
	else
	{
		Tile::harvestBlock(level, player, x, y, z, data);
	}
}
