#pragma once

#include "java/Random.h"
#include "world/level/tile/Tile.h"

// 1.0 BrewingStandBlock (117); data bits 0-2 show which bottle holders are filled
class BrewingStandTile : public Tile
{
	Random random;

public:
	explicit BrewingStandTile(int_t id);

	bool isSolidRender() override { return false; }
	bool isCubeShaped() override { return false; }
	Shape getRenderShape() override { return SHAPE_BREWING_STAND; }
	void addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList) override;
	void updateDefaultShape() override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void onRemove(Level &level, int_t x, int_t y, int_t z) override;
	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void animateTick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	int_t getResource(int_t data, Random &random) override;
};

// 1.0 CauldronBlock (118); data 0-3 is the water level
class CauldronTile : public Tile
{
public:
	explicit CauldronTile(int_t id);

	int_t getTexture(Facing face, int_t data) override;
	bool isSolidRender() override { return false; }
	bool isCubeShaped() override { return false; }
	Shape getRenderShape() override { return SHAPE_CAULDRON; }
	void addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList) override;
	void updateDefaultShape() override;
	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	int_t getResource(int_t data, Random &random) override;
};
