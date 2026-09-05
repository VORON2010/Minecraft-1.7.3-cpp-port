#include "world/level/levelgen/feature/FlowerFeature.h"

#include "world/level/Level.h"
#include "world/level/tile/FlowerTile.h"

FlowerFeature::FlowerFeature(int_t flowerId) : flowerId(flowerId) {}

bool FlowerFeature::place(Level &level, Random &random, int_t x, int_t y, int_t z) {
	FlowerTile *plant = dynamic_cast<FlowerTile *>(Tile::tiles[flowerId]);
	for (int_t i = 0; i < 64; ++i) {
		int_t xx = x + random.nextInt(8);
		xx -= random.nextInt(8);
		int_t yy = y + random.nextInt(4);
		yy -= random.nextInt(4);
		int_t zz = z + random.nextInt(8);
		zz -= random.nextInt(8);
		if (level.isEmptyTile(xx, yy, zz) && plant != nullptr && plant->canStay(level, xx, yy, zz))
			level.setTileNoUpdate(xx, yy, zz, flowerId);
	}

	return true;
}
