#pragma once

#include "world/level/tile/Tile.h"

class FireTile : public Tile
{
public:
	FireTile(int_t id, int_t tex);

	Shape getRenderShape() override;
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override;
	bool isSolidRender() override;
	bool isCubeShaped() override;
	int_t getResourceCount(Random &random) override;
	int_t getTickDelay() override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	void animateTick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	bool mayPick() override;
	bool mayPlace(Level &level, int_t x, int_t y, int_t z) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;

	static bool canBlockCatchFire(LevelSource &level, int_t x, int_t y, int_t z);
};
