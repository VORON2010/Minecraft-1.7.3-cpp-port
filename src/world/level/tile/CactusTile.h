#pragma once

#include "world/level/tile/Tile.h"

class CactusTile : public Tile
{
public:
	CactusTile(int_t id, int_t tex);

	bool isCubeShaped() override;
	Shape getRenderShape() override;
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override;
	AABB *getTileAABB(Level &level, int_t x, int_t y, int_t z) override;
	bool isSolidRender() override;
	int_t getTexture(Facing face) override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void entityInside(Level &level, int_t x, int_t y, int_t z, Entity &entity) override;

	bool canStay(Level &level, int_t x, int_t y, int_t z);
};
