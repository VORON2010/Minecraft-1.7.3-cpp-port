#pragma once

#include "world/level/tile/Tile.h"

class PistonBaseTile : public Tile
{
private:
	bool isSticky = false;
	bool suppressNeighborUpdates = false;

public:
	PistonBaseTile(int_t id, int_t tex, bool sticky);

	int_t getTexture(Facing face, int_t data) override;
	Shape getRenderShape() override;
	bool isSolidRender() override;
	bool isCubeShaped() override;
	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void setPlacedBy(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void playBlock(Level &level, int_t x, int_t y, int_t z, int_t type, int_t data) override;
	void updateShape(LevelSource &level, int_t x, int_t y, int_t z) override;
	void updateDefaultShape() override;
	void addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList) override;
	int_t getResourceCount(Random &random) override;

	static int_t getDirection(int_t data);
	static bool isPowered(int_t data);
	static bool canPushBlock(int_t tileId, Level &level, int_t x, int_t y, int_t z, bool allowBreak);

private:
	void checkState(Level &level, int_t x, int_t y, int_t z);
	static int_t getPlacementDirection(Level &level, int_t x, int_t y, int_t z, Player &player);
	static bool canExtend(Level &level, int_t x, int_t y, int_t z, int_t dir);
	bool doExtend(Level &level, int_t x, int_t y, int_t z, int_t dir);
	static bool isIndirectlyPowered(Level &level, int_t x, int_t y, int_t z, int_t dir);
};
