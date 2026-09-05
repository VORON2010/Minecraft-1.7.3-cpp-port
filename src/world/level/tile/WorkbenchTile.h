#pragma once

#include "world/level/tile/Tile.h"

class WorkbenchTile : public Tile
{
public:
	WorkbenchTile(int_t id);

	int_t getTexture(Facing face, int_t data) override;
	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
};
