#pragma once

#include "world/level/tile/TransparentTile.h"

class IceTile : public TransparentTile
{
public:
	IceTile(int_t id, int_t tex);

	int_t getRenderLayer() override;
	bool shouldRenderFace(LevelSource &level, int_t x, int_t y, int_t z, Facing face) override;
	void harvestBlock(Level &level, Player &player, int_t x, int_t y, int_t z, int_t data) override;
	int_t getResourceCount(Random &random) override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
};
