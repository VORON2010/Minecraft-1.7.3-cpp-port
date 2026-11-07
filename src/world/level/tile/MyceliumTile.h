#pragma once

#include "world/level/tile/Tile.h"

// 1.0 mycelium (110): mushroom island ground, spreads over dirt like grass
class MyceliumTile : public Tile
{
public:
	MyceliumTile(int_t id);

	int_t getTexture(LevelSource &level, int_t x, int_t y, int_t z, Facing face) override;
	int_t getTexture(Facing face, int_t data) override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	void animateTick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	int_t getResource(int_t data, Random &random) override;
};
