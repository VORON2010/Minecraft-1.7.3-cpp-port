#pragma once

#include "world/level/tile/Tile.h"

// b1.8: vines (106). data bits: 1 = south, 2 = west, 4 = north, 8 = east; 0 = hanging from the block above
class VineTile : public Tile
{
public:
	VineTile(int_t id);

	bool isCubeShaped() override;
	bool isSolidRender() override;
	Shape getRenderShape() override { return SHAPE_VINE; }
	void updateDefaultShape() override;
	void updateShape(LevelSource &level, int_t x, int_t y, int_t z) override;
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override;
	bool mayPlaceOnFace(Level &level, int_t x, int_t y, int_t z, Facing face) override;
	int_t getColor(LevelSource &level, int_t x, int_t y, int_t z) override;
	int_t getItemColor(int_t data) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	void setPlacedOnFace(Level &level, int_t x, int_t y, int_t z, Facing face) override;
	int_t getResource(int_t data, Random &random) override;
	int_t getResourceCount(Random &random) override;
	void harvestBlock(Level &level, Player &player, int_t x, int_t y, int_t z, int_t data) override;

private:
	bool canGrowOn(int_t tile);
	bool updateAttachments(Level &level, int_t x, int_t y, int_t z);
};
