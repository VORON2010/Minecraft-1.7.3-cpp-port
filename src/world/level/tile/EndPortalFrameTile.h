#pragma once

#include "world/level/tile/Tile.h"

// 1.0 end portal frame; data bits 0-1 facing, bit 2 holds an eye of ender
class EndPortalFrameTile : public Tile
{
public:
	explicit EndPortalFrameTile(int_t id);

	static bool hasEye(int_t data) { return (data & 4) != 0; }

	int_t getTexture(Facing face, int_t data) override;
	bool isSolidRender() override;
	Shape getRenderShape() override;
	void updateDefaultShape() override;
	void addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList) override;
	int_t getResourceCount(Random &random) override;
	void setPlacedBy(Level &level, int_t x, int_t y, int_t z, Player &player) override;
};
