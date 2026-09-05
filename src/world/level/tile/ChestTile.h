#pragma once

#include "world/level/tile/Tile.h"

class ChestTile : public Tile
{
public:
	explicit ChestTile(int_t id);

	int_t getTexture(LevelSource &level, int_t x, int_t y, int_t z, Facing face) override;
	int_t getTexture(Facing face) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void onRemove(Level &level, int_t x, int_t y, int_t z) override;
	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	bool mayPlace(Level &level, int_t x, int_t y, int_t z) override;

private:
	bool hasNeighborChest(Level &level, int_t x, int_t y, int_t z) const;
	bool isBlockedChest(Level &level, int_t x, int_t y, int_t z) const;
	void dropContents(Level &level, int_t x, int_t y, int_t z) const;
};
