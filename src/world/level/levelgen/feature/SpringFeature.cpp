#include "world/level/levelgen/feature/SpringFeature.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/StoneTile.h"

SpringFeature::SpringFeature(int_t blockId) : blockId(blockId) {}

bool SpringFeature::place(Level &level, Random &random, int_t x, int_t y, int_t z) {
	if (level.getTile(x, y + 1, z) != Tile::rock.id)
		return false;
	if (level.getTile(x, y - 1, z) != Tile::rock.id)
		return false;
	if (level.getTile(x, y, z) != 0 && level.getTile(x, y, z) != Tile::rock.id)
		return false;

	int_t stoneCount = 0;
	if (level.getTile(x - 1, y, z) == Tile::rock.id) ++stoneCount;
	if (level.getTile(x + 1, y, z) == Tile::rock.id) ++stoneCount;
	if (level.getTile(x, y, z - 1) == Tile::rock.id) ++stoneCount;
	if (level.getTile(x, y, z + 1) == Tile::rock.id) ++stoneCount;

	int_t airCount = 0;
	if (level.isEmptyTile(x - 1, y, z)) ++airCount;
	if (level.isEmptyTile(x + 1, y, z)) ++airCount;
	if (level.isEmptyTile(x, y, z - 1)) ++airCount;
	if (level.isEmptyTile(x, y, z + 1)) ++airCount;

	if (stoneCount == 3 && airCount == 1) {
		level.setTile(x, y, z, blockId);
		level.instaTick = true;
		Tile::tiles[blockId]->tick(level, x, y, z, random);
		level.instaTick = false;
	}

	return true;
}
