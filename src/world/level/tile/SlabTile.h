#pragma once

#include "world/level/tile/Tile.h"

class SlabTile : public Tile
{
private:
	bool fullBlock = false;

public:
	SlabTile(int_t id, bool fullBlock);

	int_t getTexture(Facing face, int_t data) override;
	int_t getTexture(Facing face) override { return getTexture(face, 0); }
	bool isCubeShaped() override;
	bool isSolidRender() override;
	bool shouldRenderFace(LevelSource &level, int_t x, int_t y, int_t z, Facing face) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	int_t getResourceCount(Random &random) override;
	int_t getResource(int_t data, Random &random) override;
	int_t getSpawnResourcesAuxValue(int_t data) override;

protected:
	void updateDefaultShape() override;
};
