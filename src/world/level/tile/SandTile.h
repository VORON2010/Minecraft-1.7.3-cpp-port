#pragma once

#include "world/level/tile/Tile.h"

class SandTile : public Tile
{
public:
	static bool fallInstantly;

	SandTile(int_t id, int_t tex);

	void tick(Level &level, int_t x, int_t y, int_t z, Random &random) override;
	void neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile) override;
	int_t getTickDelay() override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;

private:
	void checkSlide(Level &level, int_t x, int_t y, int_t z);
	static bool isFree(Level &level, int_t x, int_t y, int_t z);
};
