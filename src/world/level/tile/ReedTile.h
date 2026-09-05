#pragma once

#include "world/level/tile/Tile.h"

class ReedTile : public Tile
{
public:
	ReedTile(int_t id, int_t tex);

	bool isCubeShaped() override;
	Shape getRenderShape() override;
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override;
	bool isSolidRender() override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	int_t getResource(int_t data, Random &random) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void updateDefaultShape() override;

	bool canStay(Level &level, int_t x, int_t y, int_t z);
};
