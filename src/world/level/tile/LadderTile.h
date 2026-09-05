#pragma once

#include "world/level/tile/Tile.h"

class LadderTile : public Tile
{
private:
	void setShapeForData(int_t data);
	bool canSurvive(Level &level, int_t x, int_t y, int_t z, int_t data);
	void dropIfUnsupported(Level &level, int_t x, int_t y, int_t z);

public:
	LadderTile(int_t id, int_t tex);

	bool isCubeShaped() override;
	bool isSolidRender() override;
	Shape getRenderShape() override;
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override;
	AABB *getTileAABB(Level &level, int_t x, int_t y, int_t z) override;
	void updateShape(LevelSource &level, int_t x, int_t y, int_t z) override;
	void updateDefaultShape() override;
	void setPlacedOnFace(Level &level, int_t x, int_t y, int_t z, Facing face) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
};