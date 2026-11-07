#pragma once

#include "world/level/tile/SandTile.h"

// 1.4.7 AnvilBlock (145): falls like sand; data bits 0-1 facing, bits 2-3 damage (intact, slightly, very)
class AnvilTile : public SandTile
{
public:
	// which of the four boxes is being drawn, the top one shows the damage texture
	int_t renderPart = 0;

	explicit AnvilTile(int_t id);

	bool isCubeShaped() override { return false; }
	bool isSolidRender() override { return false; }
	int_t getTexture(Facing face, int_t data) override;
	Shape getRenderShape() override { return SHAPE_ANVIL; }
	bool shouldRenderFace(LevelSource &level, int_t x, int_t y, int_t z, Facing face) override { (void)level; (void)x; (void)y; (void)z; (void)face; return true; }
	void updateShape(LevelSource &level, int_t x, int_t y, int_t z) override;
	void setPlacedBy(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	int_t getSpawnResourcesAuxValue(int_t data) override { return data >> 2; }
	int_t getPlacedData(int_t itemData) override { return itemData << 2; }
};
