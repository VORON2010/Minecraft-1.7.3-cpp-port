#pragma once

#include "world/level/tile/Tile.h"

class NoteTile : public Tile
{
public:
	NoteTile(int_t id, int_t tex, const Material &material);

	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void onRemove(Level &level, int_t x, int_t y, int_t z) override;
	bool use(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void attack(Level &level, int_t x, int_t y, int_t z, Player &player) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
};