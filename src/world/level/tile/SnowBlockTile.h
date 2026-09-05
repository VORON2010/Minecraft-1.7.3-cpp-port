#pragma once

#include "world/level/tile/Tile.h"

class SnowBlockTile : public Tile
{
public:
	SnowBlockTile(int_t id, int_t tex, const Material &material);

	int_t getResource(int_t data, Random &random) override;
	int_t getResourceCount(Random &random) override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
};