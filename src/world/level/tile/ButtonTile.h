#pragma once

#include "world/level/tile/Tile.h"

class ButtonTile : public Tile
{
private:
	static int_t getOrientation(Level &level, int_t x, int_t y, int_t z);

public:
	ButtonTile(int_t id, int_t tex);

	bool isCubeShaped() override { return false; }
	bool isSolidRender() override { return false; }
	Shape getRenderShape() override { return SHAPE_BLOCK; }
	bool mayPick(int_t data, bool canPickLiquid) override { return true; }
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override { return nullptr; }
	int_t getTickDelay() override { return 20; }

	bool getSignal(Level &level, int_t x, int_t y, int_t z, int_t dir) override;
	bool getDirectSignal(Level &level, int_t x, int_t y, int_t z, int_t dir) override;
	bool isSignalSource() override { return true; }

	void setPlacedOnFace(Level &level, int_t x, int_t y, int_t z, Facing face) override;
	bool mayPlace(Level &level, int_t x, int_t y, int_t z) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void attack(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void onRemove(Level &level, int_t x, int_t y, int_t z) override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	void updateShape(LevelSource &level, int_t x, int_t y, int_t z) override;
	void updateDefaultShape() override;
};