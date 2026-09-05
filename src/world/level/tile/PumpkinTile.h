#pragma once

#include "world/level/tile/Tile.h"

class PumpkinTile : public Tile
{
private:
	bool lit = false;

public:
	PumpkinTile(int_t id, int_t tex, bool lit);

	int_t getTexture(Facing face, int_t data) override;
};