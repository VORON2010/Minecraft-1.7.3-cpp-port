#pragma once

#include "world/level/tile/Tile.h"

class ChestTile : public Tile
{
public:
	explicit ChestTile(int_t id);

	// 1.4.7: a model drawn by the chest renderer, a little smaller than a block
	bool isSolidRender() override { return false; }
	bool isCubeShaped() override { return false; }
	Shape getRenderShape() override { return SHAPE_ENTITYBLOCK_ANIMATED; }
	void updateShape(LevelSource &level, int_t x, int_t y, int_t z) override;
	void updateDefaultShape() override;
	void setPlacedBy(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	void updateState(Level &level, int_t x, int_t y, int_t z);

	int_t getTexture(LevelSource &level, int_t x, int_t y, int_t z, Facing face) override;
	int_t getTexture(Facing face) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void onRemove(Level &level, int_t x, int_t y, int_t z) override;
	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	bool mayPlace(Level &level, int_t x, int_t y, int_t z) override;

private:
	int_t legacyTexture(LevelSource &level, int_t x, int_t y, int_t z, Facing face);
	bool hasNeighborChest(Level &level, int_t x, int_t y, int_t z) const;
	bool isBlockedChest(Level &level, int_t x, int_t y, int_t z) const;
	void dropContents(Level &level, int_t x, int_t y, int_t z) const;
};
