#include "world/level/levelgen/feature/ForestFeature.h"

#include "world/level/Level.h"

#include "world/level/tile/Tile.h"
#include "world/level/tile/GrassTile.h"
#include "world/level/tile/DirtTile.h"
#include "world/level/tile/TreeTile.h"
#include "world/level/tile/LeafTile.h"

#include <cmath>

bool ForestFeature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	int_t height = random.nextInt(3) + 5;
	bool canPlace = true;
	if (y < 1 || y + height + 1 > Level::DEPTH)
		return false;

	for (int_t yy = y; yy <= y + 1 + height; ++yy)
	{
		int_t radius = 1;
		if (yy == y)
			radius = 0;
		if (yy >= y + 1 + height - 2)
			radius = 2;

		for (int_t xx = x - radius; xx <= x + radius && canPlace; ++xx)
		{
			for (int_t zz = z - radius; zz <= z + radius && canPlace; ++zz)
			{
				if (yy >= 0 && yy < Level::DEPTH)
				{
					int_t tile = level.getTile(xx, yy, zz);
					if (tile != 0 && tile != Tile::leaves.id)
						canPlace = false;
				}
				else
				{
					canPlace = false;
				}
			}
		}
	}

	if (!canPlace)
		return false;

	int_t soil = level.getTile(x, y - 1, z);
	if ((soil != Tile::grass.id && soil != Tile::dirt.id) || y >= Level::DEPTH - height - 1)
		return false;

	level.setTileNoUpdate(x, y - 1, z, Tile::dirt.id);

	for (int_t yy = y - 3 + height; yy <= y + height; ++yy)
	{
		int_t offset = yy - (y + height);
		int_t radius = 1 - offset / 2;
		for (int_t xx = x - radius; xx <= x + radius; ++xx)
		{
			int_t dx = xx - x;
			for (int_t zz = z - radius; zz <= z + radius; ++zz)
			{
				int_t dz = zz - z;
				if ((std::abs(dx) != radius || std::abs(dz) != radius || (random.nextInt(2) != 0 && offset != 0)) && !Tile::solid[level.getTile(xx, yy, zz)])
					level.setTileAndDataNoUpdate(xx, yy, zz, Tile::leaves.id, LeafTile::BIRCH_LEAF);
			}
		}
	}

	for (int_t yy = 0; yy < height; ++yy)
	{
		int_t tile = level.getTile(x, y + yy, z);
		if (tile == 0 || tile == Tile::leaves.id)
			level.setTileAndDataNoUpdate(x, y + yy, z, Tile::treeTrunk.id, TreeTile::BIRCH_TRUNK);
	}

	return true;
}
