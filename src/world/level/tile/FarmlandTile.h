#pragma once

#include "world/level/tile/Tile.h"

class FarmlandTile : public Tile
{
public:
	FarmlandTile(int_t id);

	bool isCubeShaped() override;
	int_t getTexture(Facing face, int_t data) override;
	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override;
	bool isSolidRender() override;
	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	int_t getResource(int_t data, Random &random) override;
	void stepOn(Level &level, int_t x, int_t y, int_t z, Entity &entity) override;
	void updateDefaultShape() override;

private:
	bool isWaterNearby(Level &level, int_t x, int_t y, int_t z) const;
	bool hasCropAbove(Level &level, int_t x, int_t y, int_t z) const;
};
