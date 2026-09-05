#pragma once

#include "world/level/tile/Tile.h"

class GlowStoneTile : public Tile
{
public:
	GlowStoneTile(int_t id, int_t tex, const Material &material);

	int_t getResource(int_t data, Random &random) override;
	int_t getResourceCount(Random &random) override;
};