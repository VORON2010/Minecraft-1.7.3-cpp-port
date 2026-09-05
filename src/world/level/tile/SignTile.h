#pragma once

#include "world/level/tile/Tile.h"

class SignTile : public Tile
{
private:
	bool freestanding;

public:
	SignTile(int_t id, bool freestanding);

	bool isSolidRender() override;
	bool isCubeShaped() override;
	Shape getRenderShape() override;
	int_t getResource(int_t data, Random &random) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void onRemove(Level &level, int_t x, int_t y, int_t z) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t id) override;
	AABB *getTileAABB(Level &level, int_t x, int_t y, int_t z) override;
	void updateShape(LevelSource &level, int_t x, int_t y, int_t z) override;
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override;
};