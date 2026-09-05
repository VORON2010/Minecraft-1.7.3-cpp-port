#pragma once

#include "world/level/tile/TransparentTile.h"

class PortalTile : public TransparentTile
{
public:
	PortalTile(int_t id, int_t tex);

	AABB *getAABB(Level &level, int_t x, int_t y, int_t z) override;
	void updateShape(LevelSource &level, int_t x, int_t y, int_t z) override;
	bool isCubeShaped() override;
	bool shouldRenderFace(LevelSource &level, int_t x, int_t y, int_t z, Facing face) override;
	int_t getResourceCount(Random &random) override;
	int_t getRenderLayer() override;
	void entityInside(Level &level, int_t x, int_t y, int_t z, Entity &entity) override;
	void animateTick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;

	bool trySpawnPortal(Level &level, int_t x, int_t y, int_t z);
};
