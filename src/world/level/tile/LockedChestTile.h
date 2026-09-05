#pragma once

#include "world/level/tile/Tile.h"

class LockedChestTile : public Tile
{
public:
	explicit LockedChestTile(int_t id);
	int_t getTexture(LevelSource &level, int_t x, int_t y, int_t z, Facing face) override;
	int_t getTexture(Facing face, int_t data) override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
};
