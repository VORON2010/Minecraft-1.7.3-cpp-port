#pragma once

#include "world/level/tile/Tile.h"

class SoulSandTile : public Tile
{
public:
	SoulSandTile(int_t id, int_t tex, const Material &material);

	void entityInside(Level &level, int_t x, int_t y, int_t z, Entity &entity) override;
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override;
};