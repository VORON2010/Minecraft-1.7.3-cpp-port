#pragma once

#include "world/level/tile/Tile.h"

class SandStoneTile : public Tile
{
public:
	SandStoneTile(int_t id, int_t texture);
	int_t getTexture(Facing face) override;
};
