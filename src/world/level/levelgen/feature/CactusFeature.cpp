#include "world/level/levelgen/feature/CactusFeature.h"

#include "world/level/Level.h"
#include "world/level/tile/CactusTile.h"
#include "world/level/tile/Tile.h"

bool CactusFeature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	for (int_t attempt = 0; attempt < 10; ++attempt)
	{
		int_t placeX = x + random.nextInt(8);
		placeX -= random.nextInt(8);
		int_t placeY = y + random.nextInt(4);
		placeY -= random.nextInt(4);
		int_t placeZ = z + random.nextInt(8);
		placeZ -= random.nextInt(8);
		if (!level.isEmptyTile(placeX, placeY, placeZ))
			continue;

		int_t height = 1 + random.nextInt(random.nextInt(3) + 1);
		for (int_t yy = 0; yy < height; ++yy)
		{
			int_t stackY = placeY + yy;
			if (Tile::cactus.canStay(level, placeX, stackY, placeZ))
				level.setTileNoUpdate(placeX, stackY, placeZ, Tile::cactus.id);
		}
	}

	return true;
}
