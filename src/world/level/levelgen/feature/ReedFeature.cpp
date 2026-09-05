#include "world/level/levelgen/feature/ReedFeature.h"

#include "world/level/Level.h"
#include "world/level/tile/LiquidTile.h"
#include "world/level/tile/ReedTile.h"
#include "world/level/tile/Tile.h"

bool ReedFeature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	for (int_t attempt = 0; attempt < 20; ++attempt)
	{
		int_t placeX = x + random.nextInt(4);
		placeX -= random.nextInt(4);
		int_t placeZ = z + random.nextInt(4);
		placeZ -= random.nextInt(4);
		if (!level.isEmptyTile(placeX, y, placeZ))
			continue;

		bool besideWater =
			level.getTile(placeX - 1, y - 1, placeZ) == Tile::water.id ||
			level.getTile(placeX - 1, y - 1, placeZ) == Tile::calmWater.id ||
			level.getTile(placeX + 1, y - 1, placeZ) == Tile::water.id ||
			level.getTile(placeX + 1, y - 1, placeZ) == Tile::calmWater.id ||
			level.getTile(placeX, y - 1, placeZ - 1) == Tile::water.id ||
			level.getTile(placeX, y - 1, placeZ - 1) == Tile::calmWater.id ||
			level.getTile(placeX, y - 1, placeZ + 1) == Tile::water.id ||
			level.getTile(placeX, y - 1, placeZ + 1) == Tile::calmWater.id;
		if (!besideWater)
			continue;

		int_t height = 2 + random.nextInt(random.nextInt(3) + 1);
		for (int_t yy = 0; yy < height; ++yy)
		{
			int_t placeY = y + yy;
			if (Tile::reed.canStay(level, placeX, placeY, placeZ))
				level.setTileNoUpdate(placeX, placeY, placeZ, Tile::reed.id);
		}
	}

	return true;
}
