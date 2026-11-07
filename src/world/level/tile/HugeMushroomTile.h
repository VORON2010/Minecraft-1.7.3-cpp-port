#pragma once

#include "world/level/tile/Tile.h"

// b1.8: huge mushroom blocks (99 brown, 100 red). data picks which faces show cap/stem
class HugeMushroomTile : public Tile
{
private:
	int_t variant = 0;

public:
	HugeMushroomTile(int_t id, int_t tex, int_t variant);

	int_t getTexture(Facing face, int_t data) override;
	int_t getResourceCount(Random &random) override;
	int_t getResource(int_t data, Random &random) override;
};
