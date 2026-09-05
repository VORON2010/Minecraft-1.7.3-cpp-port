#pragma once

#include "world/level/tile/Tile.h"

class LeverTile : public Tile
{
public:
	LeverTile(int_t id, int_t tex);

	bool isCubeShaped() override { return false; }
	bool isSolidRender() override { return false; }
	Shape getRenderShape() override { return SHAPE_LEVER; }
	bool mayPick(int_t data, bool canPickLiquid) override { return true; }
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override { return nullptr; }

	bool getSignal(Level &level, int_t x, int_t y, int_t z, int_t dir) override;
	bool getDirectSignal(Level &level, int_t x, int_t y, int_t z, int_t dir) override;
	bool isSignalSource() override { return true; }

	void setPlacedOnFace(Level &level, int_t x, int_t y, int_t z, Facing face) override;
	bool canPlaceOnSide(Level &level, int_t x, int_t y, int_t z, Facing face);
	bool mayPlace(Level &level, int_t x, int_t y, int_t z) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void attack(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void onRemove(Level &level, int_t x, int_t y, int_t z) override;
	void updateShape(LevelSource &level, int_t x, int_t y, int_t z) override;

private:
	bool checkCanSurvive(Level &level, int_t x, int_t y, int_t z);
};