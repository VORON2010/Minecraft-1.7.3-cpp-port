#include "world/level/tile/SaplingTile.h"

#include "world/item/Item.h"
#include "world/item/Items.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/level/levelgen/feature/TreeFeature.h"
#include "world/level/levelgen/feature/BigTreeFeature.h"
#include "world/level/levelgen/feature/ForestFeature.h"
#include "world/level/levelgen/feature/Taiga2Feature.h"

SaplingTile::SaplingTile(int_t id, int_t tex) : FlowerTile(id, tex)
{
	float radius = 0.4f;
	setShape(0.5f - radius, 0.0f, 0.5f - radius, 0.5f + radius, radius * 2.0f, 0.5f + radius);
}

void SaplingTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	if (!canStay(level, x, y, z))
	{
		level.setTile(x, y, z, 0);
		return;
	}

	if (level.getRawBrightness(x, y + 1, z) >= 9 && random.nextInt(30) == 0)
	{
		int_t data = level.getData(x, y, z);
		if ((data & 8) == 0)
		{
			level.setData(x, y, z, data | 8);
		}
		else
		{
			growTree(level, x, y, z, random);
		}
	}
}

void SaplingTile::growTree(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	int_t type = level.getData(x, y, z) & 3;
	level.setTile(x, y, z, 0);

	bool placed = false;
	if (type == 1)
	{
		Taiga2Feature gen;
		placed = gen.place(level, random, x, y, z);
	}
	else if (type == 2)
	{
		ForestFeature gen;
		placed = gen.place(level, random, x, y, z);
	}
	else
	{
		if (random.nextInt(10) == 0)
		{
			BigTreeFeature gen;
			placed = gen.place(level, random, x, y, z);
		}
		if (!placed)
		{
			TreeFeature gen;
			placed = gen.place(level, random, x, y, z);
		}
	}

	if (!placed)
		level.setTileAndData(x, y, z, id, type);
}

int_t SaplingTile::getTexture(Facing face, int_t data)
{
	int_t type = data & 3;
	if (type == 1) return 63;
	if (type == 2) return 79;
	return tex;
}

int_t SaplingTile::getResource(int_t data, Random &random)
{
	(void)data;
	(void)random;
	return id;
}

int_t SaplingTile::getSpawnResourcesAuxValue(int_t data)
{
	return data & 3;
}

bool SaplingTile::canSurviveOn(int_t belowTile) const
{
	return belowTile == 2 || belowTile == 3 || belowTile == 60;
}