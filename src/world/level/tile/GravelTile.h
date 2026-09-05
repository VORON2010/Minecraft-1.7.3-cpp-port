#pragma once

#include "world/level/tile/SandTile.h"

class GravelTile : public SandTile
{
public:
	GravelTile(int_t id, int_t tex);
	int_t getResource(int_t data, Random &random) override;
};
