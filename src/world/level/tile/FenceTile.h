#pragma once

#include "world/level/tile/Tile.h"

class FenceTile : public Tile
{
public:
	FenceTile(int_t id, int_t tex, const Material &material);

	bool isCubeShaped() override;
	bool isSolidRender() override;
	Shape getRenderShape() override;
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override;
};