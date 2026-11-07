#pragma once

#include "world/level/tile/Tile.h"

// 1.0 EndPortalBlock: thin glowing slab, drawn by its tile entity renderer
class EndPortalTile : public Tile
{
public:
	static bool forcePlace;

	EndPortalTile(int_t id, const Material &material);

	void updateShape(LevelSource &level, int_t x, int_t y, int_t z) override;
	bool shouldRenderFace(LevelSource &level, int_t x, int_t y, int_t z, Facing face) override;
	void addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList) override;
	bool isSolidRender() override;
	bool isCubeShaped() override;
	int_t getResourceCount(Random &random) override;
	void entityInside(Level &level, int_t x, int_t y, int_t z, Entity &entity) override;
	void animateTick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	Shape getRenderShape() override { return SHAPE_INVISIBLE; }
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void onRemove(Level &level, int_t x, int_t y, int_t z) override;
};
