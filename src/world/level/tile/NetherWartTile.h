#pragma once

#include "world/level/tile/FlowerTile.h"

// 1.0 nether wart: grows on soul sand, only in the nether biome
class NetherWartTile : public FlowerTile
{
public:
	explicit NetherWartTile(int_t id);

	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	int_t getTexture(Facing face, int_t data) override;
	Shape getRenderShape() override { return SHAPE_ROWS; }
	void spawnResources(Level &level, int_t x, int_t y, int_t z, int_t data, float chance) override;
	int_t getResource(int_t data, Random &random) override;
	int_t getResourceCount(Random &random) override;
	void updateDefaultShape() override;

protected:
	bool canSurviveOn(int_t belowTile) const override;
};
