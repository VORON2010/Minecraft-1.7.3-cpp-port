#pragma once

#include "world/level/tile/FlowerTile.h"

class CropsTile : public FlowerTile
{
public:
	CropsTile(int_t id, int_t tex);

	int_t getTexture(Facing face, int_t data) override;
	Shape getRenderShape() override { return SHAPE_ROWS; }
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	int_t getResource(int_t data, Random &random) override;
	void spawnResources(Level &level, int_t x, int_t y, int_t z, int_t data, float chance) override;

protected:
	bool canSurviveOn(int_t belowTile) const override;

private:
	float getGrowthSpeed(Level &level, int_t x, int_t y, int_t z);
	void spawnSeed(Level &level, int_t x, int_t y, int_t z);
};
