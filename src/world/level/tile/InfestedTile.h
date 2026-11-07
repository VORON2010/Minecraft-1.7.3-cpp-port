#pragma once

#include "world/level/tile/Tile.h"

// b1.8: monster egg (97), stone with a silverfish inside.
// data 0 = stone, 1 = cobblestone, 2 = stone bricks
class InfestedTile : public Tile
{
public:
	InfestedTile(int_t id);

	int_t getTexture(Facing face, int_t data) override;
	void destroy(Level &level, int_t x, int_t y, int_t z, int_t data) override;
	int_t getResourceCount(Random &random) override;

	static bool canBeInfested(int_t tile);
	static int_t getVariant(int_t tile);
};
