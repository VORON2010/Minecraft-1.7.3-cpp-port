#pragma once

#include "world/level/tile/Tile.h"

// b1.8: iron bars (101) and glass pane (102)
class PaneTile : public Tile
{
private:
	int_t topTex = 0;

public:
	PaneTile(int_t id, int_t tex, int_t topTex, const Material &material);

	bool isCubeShaped() override;
	bool isSolidRender() override;
	Shape getRenderShape() override;
	bool shouldRenderFace(LevelSource &level, int_t x, int_t y, int_t z, Facing face) override;
	void addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList) override;
	void updateDefaultShape() override;
	void updateShape(LevelSource &level, int_t x, int_t y, int_t z) override;

	int_t getTopTexture() const { return topTex; }
	bool attachesTo(int_t tile) const;
};
