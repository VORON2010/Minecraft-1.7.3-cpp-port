#pragma once

#include "world/level/tile/Tile.h"

class SnowTile : public Tile
{
public:
	SnowTile(int_t id, int_t tex);

	bool isCubeShaped() override;
	bool isSolidRender() override;
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void updateShape(LevelSource &level, int_t x, int_t y, int_t z) override;
	void updateDefaultShape() override;
	void harvestBlock(Level &level, Player &player, int_t x, int_t y, int_t z, int_t data) override;
	int_t getResource(int_t data, Random &random) override;
	int_t getResourceCount(Random &random) override;
	bool shouldRenderFace(LevelSource &level, int_t x, int_t y, int_t z, Facing face) override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;

private:
	bool canSnowStay(LevelSource &level, int_t x, int_t y, int_t z);
};
