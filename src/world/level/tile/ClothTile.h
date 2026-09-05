#pragma once

#include "world/level/tile/Tile.h"

class ClothTile : public Tile
{
public:
	ClothTile(int_t id, int_t tex, const Material &material);

	int_t getTexture(Facing face, int_t data) override;
	int_t getSpawnResourcesAuxValue(int_t data) override;
};