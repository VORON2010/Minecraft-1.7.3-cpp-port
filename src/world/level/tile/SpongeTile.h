#pragma once

#include "world/level/tile/Tile.h"

class SpongeTile : public Tile
{
public:
	SpongeTile(int_t id, int_t tex, const Material &material);

	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void onRemove(Level &level, int_t x, int_t y, int_t z) override;
};