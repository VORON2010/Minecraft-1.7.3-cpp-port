#include "world/level/levelgen/feature/PumpkinFeature.h"

#include "world/level/Level.h"
#include "world/level/tile/GrassTile.h"
#include "world/level/tile/PumpkinTile.h"
#include "world/level/tile/Tile.h"

bool PumpkinFeature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	for (int_t attempt = 0; attempt < 64; ++attempt)
	{
		int_t placeX = x + random.nextInt(8);
		placeX -= random.nextInt(8);
		int_t placeY = y + random.nextInt(4);
		placeY -= random.nextInt(4);
		int_t placeZ = z + random.nextInt(8);
		placeZ -= random.nextInt(8);
		if (!level.isEmptyTile(placeX, placeY, placeZ))
			continue;
		if (level.getTile(placeX, placeY - 1, placeZ) != Tile::grass.id)
			continue;

		level.setTileAndDataNoUpdate(placeX, placeY, placeZ, Tile::pumpkin.id, random.nextInt(4));
	}

	return true;
}
