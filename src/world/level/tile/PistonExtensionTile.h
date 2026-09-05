#pragma once

#include "world/level/tile/Tile.h"

class PistonExtensionTile : public Tile
{
private:
	int_t headTextureOverride = -1;

public:
	PistonExtensionTile(int_t id, int_t tex);

	void setHeadTexture(int_t tex);
	void clearHeadTexture();

	void onRemove(Level &level, int_t x, int_t y, int_t z) override;
	int_t getTexture(Facing face, int_t data) override;
	Shape getRenderShape() override;
	bool isSolidRender() override;
	bool isCubeShaped() override { return false; }
	bool mayPlace(Level &level, int_t x, int_t y, int_t z) override;
	void setPlacedOnFace(Level &level, int_t x, int_t y, int_t z, Facing face) override;
	int_t getResourceCount(Random &random) override;
	void updateShape(LevelSource &level, int_t x, int_t y, int_t z) override;
	void addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;

	static int_t getDirection(int_t data);
};