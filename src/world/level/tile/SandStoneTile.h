#pragma once

#include "world/level/tile/Tile.h"

class SandStoneTile : public Tile
{
public:
	SandStoneTile(int_t id, int_t texture);
	int_t getTexture(Facing face) override;
	// 1.4.7 SandstoneBlock: plain, chiseled (1), smooth (2)
	int_t getTexture(Facing face, int_t data) override;
	int_t getSpawnResourcesAuxValue(int_t data) override { return data; }
};
