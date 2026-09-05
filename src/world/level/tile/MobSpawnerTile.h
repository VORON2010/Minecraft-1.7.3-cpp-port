#pragma once

#include "world/level/tile/Tile.h"

class MobSpawnerTile : public Tile
{
public:
	MobSpawnerTile(int_t id, int_t tex);

	bool isSolidRender() override;
	int_t getResource(int_t data, Random &random) override;
	int_t getResourceCount(Random &random) override;
	void onPlace(Level &level, int_t x, int_t y, int_t z) override;
	void onRemove(Level &level, int_t x, int_t y, int_t z) override;
};
