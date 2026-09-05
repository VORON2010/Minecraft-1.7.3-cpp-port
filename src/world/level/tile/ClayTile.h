#pragma once

#include "world/level/tile/Tile.h"

class ClayTile : public Tile
{
public:
	ClayTile(int_t id, int_t tex);
	int_t getResource(int_t data, Random &random) override;
	int_t getResourceCount(Random &random) override;
};
