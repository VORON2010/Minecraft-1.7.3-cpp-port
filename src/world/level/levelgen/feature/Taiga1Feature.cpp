#include "world/level/levelgen/feature/Taiga1Feature.h"

#include "world/level/Level.h"

#include "world/level/tile/Tile.h"
#include "world/level/tile/GrassTile.h"
#include "world/level/tile/DirtTile.h"
#include "world/level/tile/TreeTile.h"
#include "world/level/tile/LeafTile.h"

#include <cstdlib>

bool Taiga1Feature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	int_t height = random.nextInt(5) + 7;
	int_t trunkHeight = height - random.nextInt(2) - 3;
	int_t leafHeight = height - trunkHeight;
	int_t maxLeafRadius = 1 + random.nextInt(leafHeight + 1);

	if (y < 1 || y + height + 1 > Level::DEPTH)
		return false;

	bool canPlace = true;
	for (int_t yy = y; yy <= y + 1 + height && canPlace; ++yy)
	{
		int_t radius = (yy - y < trunkHeight) ? 0 : maxLeafRadius;
		for (int_t xx = x - radius; xx <= x + radius && canPlace; ++xx)
		{
			for (int_t zz = z - radius; zz <= z + radius && canPlace; ++zz)
			{
				if (yy < 0 || yy >= Level::DEPTH)
				{
					canPlace = false;
					continue;
				}

				int_t tile = level.getTile(xx, yy, zz);
				if (tile != 0 && tile != Tile::leaves.id)
					canPlace = false;
			}
		}
	}

	if (!canPlace)
		return false;

	int_t belowTile = level.getTile(x, y - 1, z);
	if ((belowTile != Tile::grass.id && belowTile != Tile::dirt.id) || y >= Level::DEPTH - height - 1)
		return false;

	level.setTileNoUpdate(x, y - 1, z, Tile::dirt.id);

	int_t radius = 0;
	for (int_t yy = y + height; yy >= y + trunkHeight; --yy)
	{
		for (int_t xx = x - radius; xx <= x + radius; ++xx)
		{
			int_t dx = xx - x;
			for (int_t zz = z - radius; zz <= z + radius; ++zz)
			{
				int_t dz = zz - z;
				if ((std::abs(dx) != radius || std::abs(dz) != radius || radius <= 0) && !Tile::solid[level.getTile(xx, yy, zz)])
				{
					level.setTileAndDataNoUpdate(xx, yy, zz, Tile::leaves.id, LeafTile::SPRUCE_LEAF);
				}
			}
		}

		if (radius >= 1 && yy == y + trunkHeight + 1)
		{
			--radius;
		}
		else if (radius < maxLeafRadius)
		{
			++radius;
		}
	}

	for (int_t yy = 0; yy < height - 1; ++yy)
	{
		int_t tile = level.getTile(x, y + yy, z);
		if (tile == 0 || tile == Tile::leaves.id)
			level.setTileAndDataNoUpdate(x, y + yy, z, Tile::treeTrunk.id, TreeTile::SPRUCE_TRUNK);
	}

	return true;
}
