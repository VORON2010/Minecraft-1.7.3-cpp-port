#pragma once

#include "world/level/tile/FlowerTile.h"

// 1.0 lily pad (111): sits on still water
class WaterlilyTile : public FlowerTile
{
public:
	WaterlilyTile(int_t id, int_t tex);

	Shape getRenderShape() override { return SHAPE_LILYPAD; }
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override;
	int_t getColor(LevelSource &level, int_t x, int_t y, int_t z) override;
	int_t getItemColor(int_t data) override;
	bool canStay(Level &level, int_t x, int_t y, int_t z) override;
	void updateDefaultShape() override;

protected:
	bool canSurviveOn(int_t belowTile) const override;
};
