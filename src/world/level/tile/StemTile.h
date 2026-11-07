#pragma once

#include "world/level/tile/FlowerTile.h"

// b1.8: pumpkin stem (104) and melon stem (105). data 0..7 = growth stage
class StemTile : public FlowerTile
{
private:
	Tile &fruit;

public:
	StemTile(int_t id, Tile &fruit);

	Shape getRenderShape() override { return SHAPE_STEM; }
	int_t getTexture(Facing face, int_t data) override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	int_t getColor(LevelSource &level, int_t x, int_t y, int_t z) override;
	int_t getItemColor(int_t data) override;
	void updateDefaultShape() override;
	void updateShape(LevelSource &level, int_t x, int_t y, int_t z) override;
	int_t getResource(int_t data, Random &random) override;
	void spawnResources(Level &level, int_t x, int_t y, int_t z, int_t data, float chance) override;

	void growFully(Level &level, int_t x, int_t y, int_t z);
	int_t getConnectDirection(LevelSource &level, int_t x, int_t y, int_t z);
	double getHeight() const { return yy1; }

protected:
	bool canSurviveOn(int_t belowTile) const override;

private:
	float getGrowthSpeed(Level &level, int_t x, int_t y, int_t z);
};
