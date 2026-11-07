#pragma once

#include "world/level/tile/Tile.h"

// b1.8: melon block (103)
class MelonTile : public Tile
{
public:
	MelonTile(int_t id);

	int_t getTexture(Facing face, int_t data) override;
	int_t getTexture(Facing face) override;
	int_t getResource(int_t data, Random &random) override;
	int_t getResourceCount(Random &random) override;
	int_t getResourceCountForLootBonus(int_t bonus, Random &random) override;
};
