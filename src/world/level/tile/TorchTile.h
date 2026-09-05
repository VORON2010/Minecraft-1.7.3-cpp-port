#pragma once
#include "world/level/tile/Tile.h"

class TorchTile : public Tile
{
public:
	TorchTile(int_t id, int_t tex);
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override;
	bool isSolidRender() override;
	bool isCubeShaped() override;
	Shape getRenderShape() override;
	bool mayPlace(Level &level, int_t x, int_t y, int_t z);
	void setPlacedOnFace(Level &level, int_t x, int_t y, int_t z, Facing face) override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	HitResult clip(Level &level, int_t x, int_t y, int_t z, Vec3 &from, Vec3 &to) override;
	void animateTick(Level &level, int_t x, int_t y, int_t z, Random &random) override;

private:
	bool checkCanSurvive(Level &level, int_t x, int_t y, int_t z);
};