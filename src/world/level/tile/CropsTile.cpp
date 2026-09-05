#include "world/level/tile/CropsTile.h"

#include <memory>

#include "world/entity/item/EntityItem.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/level/Level.h"

namespace
{
	constexpr int_t FARMLAND_TILE_ID = 60;
}

CropsTile::CropsTile(int_t id, int_t tex) : FlowerTile(id, tex)
{
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.25f, 1.0f);
}

int_t CropsTile::getTexture(Facing face, int_t data)
{
	(void)face;
	if (data < 0)
		data = 7;
	return tex + data;
}

void CropsTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	FlowerTile::tick(level, x, y, z, random);
	if (level.getTile(x, y, z) != id)
		return;
	if (level.getRawBrightness(x, y + 1, z) < 9)
		return;

	int_t data = level.getData(x, y, z);
	if (data >= 7)
		return;

	float growthSpeed = getGrowthSpeed(level, x, y, z);
	int_t growthDelay = static_cast<int_t>(100.0f / growthSpeed);
	if (growthDelay < 1)
		growthDelay = 1;
	if (random.nextInt(growthDelay) == 0)
		level.setData(x, y, z, data + 1);
}

int_t CropsTile::getResource(int_t data, Random &random)
{
	(void)random;
	return data == 7 ? Items::wheat->getShiftedIndex() : -1;
}

void CropsTile::spawnResources(Level &level, int_t x, int_t y, int_t z, int_t data, float chance)
{
	Tile::spawnResources(level, x, y, z, data, chance);
	if (level.isOnline)
		return;

	for (int_t i = 0; i < 3; ++i)
	{
		if (level.random.nextInt(15) <= data)
			spawnSeed(level, x, y, z);
	}
}

bool CropsTile::canSurviveOn(int_t belowTile) const
{
	return belowTile == FARMLAND_TILE_ID;
}

float CropsTile::getGrowthSpeed(Level &level, int_t x, int_t y, int_t z)
{
	float growthSpeed = 1.0f;
	int_t north = level.getTile(x, y, z - 1);
	int_t south = level.getTile(x, y, z + 1);
	int_t west = level.getTile(x - 1, y, z);
	int_t east = level.getTile(x + 1, y, z);
	int_t northWest = level.getTile(x - 1, y, z - 1);
	int_t northEast = level.getTile(x + 1, y, z - 1);
	int_t southEast = level.getTile(x + 1, y, z + 1);
	int_t southWest = level.getTile(x - 1, y, z + 1);
	bool adjacentX = west == id || east == id;
	bool adjacentZ = north == id || south == id;
	bool adjacentDiagonal = northWest == id || northEast == id || southEast == id || southWest == id;

	for (int_t xx = x - 1; xx <= x + 1; ++xx)
	{
		for (int_t zz = z - 1; zz <= z + 1; ++zz)
		{
			int_t belowTile = level.getTile(xx, y - 1, zz);
			float contribution = 0.0f;
			if (belowTile == FARMLAND_TILE_ID)
			{
				contribution = 1.0f;
				if (level.getData(xx, y - 1, zz) > 0)
					contribution = 3.0f;
			}
			if (xx != x || zz != z)
				contribution /= 4.0f;
			growthSpeed += contribution;
		}
	}

	if (adjacentDiagonal || (adjacentX && adjacentZ))
		growthSpeed /= 2.0f;
	return growthSpeed;
}

void CropsTile::spawnSeed(Level &level, int_t x, int_t y, int_t z)
{
	float spread = 0.7f;
	float xo = level.random.nextFloat() * spread + (1.0f - spread) * 0.5f;
	float yo = level.random.nextFloat() * spread + (1.0f - spread) * 0.5f;
	float zo = level.random.nextFloat() * spread + (1.0f - spread) * 0.5f;
	ItemInstance stack(Items::seeds->getShiftedIndex(), 1, 0);
	auto entity = std::make_shared<EntityItem>(level, x + xo, y + yo, z + zo, stack);
	entity->throwTime = 10;
	level.addEntity(entity);
}
