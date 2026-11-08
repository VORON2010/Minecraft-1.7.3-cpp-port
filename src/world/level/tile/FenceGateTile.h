#pragma once

#include "world/level/tile/Tile.h"

// b1.8: fence gate (107). data & 3 = facing, data & 4 = open
class FenceGateTile : public Tile
{
public:
	bool isPathfindable(LevelSource &level, int_t x, int_t y, int_t z) override;
	FenceGateTile(int_t id, int_t tex);

	bool mayPlace(Level &level, int_t x, int_t y, int_t z) override;
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override;
	bool isCubeShaped() override;
	bool isSolidRender() override;
	Shape getRenderShape() override { return SHAPE_FENCE_GATE; }
	void setPlacedBy(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;

	static bool isOpen(int_t data) { return (data & 4) != 0; }
	static int_t getFacing(int_t data) { return data & 3; }
};
