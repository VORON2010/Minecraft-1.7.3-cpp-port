#pragma once

#include "world/level/tile/Tile.h"

// b1.8: stone bricks (98). data 0 = normal, 1 = mossy, 2 = cracked
class StoneBrickTile : public Tile
{
public:
	static constexpr int_t TEX_NORMAL = 54;
	static constexpr int_t TEX_MOSSY = 100;
	static constexpr int_t TEX_CRACKED = 101;

	StoneBrickTile(int_t id);

	int_t getTexture(Facing face, int_t data) override;
	int_t getSpawnResourcesAuxValue(int_t data) override { return data; }
};
