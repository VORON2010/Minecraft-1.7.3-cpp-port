#pragma once

#include "world/level/tile/Tile.h"

class WoodTile : public Tile
{
public:
	WoodTile(int_t id, int_t tex);
	using Tile::getTexture;
	// 1.4.7 PlanksBlock: oak, spruce, birch, jungle
	int_t getTexture(Facing face, int_t data) override;
	int_t getSpawnResourcesAuxValue(int_t data) override { return data; }
};
