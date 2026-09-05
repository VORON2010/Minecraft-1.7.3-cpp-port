#pragma once

#include "world/level/tile/TransparentTile.h"

class GlassTile : public TransparentTile
{
public:
	GlassTile(int_t id, int_t tex, const Material &material, bool allowSame);

	int_t getResourceCount(Random &random) override;
};