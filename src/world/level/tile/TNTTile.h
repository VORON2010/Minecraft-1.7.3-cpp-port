#pragma once

#include "world/level/tile/Tile.h"

class TNTTile : public Tile
{
public:
	TNTTile(int_t id, int_t tex);

	int_t getTexture(Facing face, int_t data) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	void attack(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	int_t getResourceCount(Random &random) override;
	void playerDestroy(Level &level, int_t x, int_t y, int_t z, int_t data) override;
	void onBlockDestroyedByExplosion(Level &level, int_t x, int_t y, int_t z) override;
};
