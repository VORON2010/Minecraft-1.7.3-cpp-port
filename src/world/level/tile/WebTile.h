#pragma once

#include "world/level/tile/Tile.h"

class WebTile : public Tile
{
public:
	WebTile(int_t id, int_t tex, const Material &material);

	bool isCubeShaped() override;
	Shape getRenderShape() override;
	bool isSolidRender() override;
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override;
	void entityInside(Level &level, int_t x, int_t y, int_t z, Entity &entity) override;
	int_t getResource(int_t data, Random &random) override;
};