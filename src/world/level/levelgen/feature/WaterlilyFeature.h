#pragma once

#include "world/level/levelgen/feature/Feature.h"

// 1.0 lily pads in swamps
class WaterlilyFeature : public Feature
{
public:
	bool place(Level &level, Random &random, int_t x, int_t y, int_t z) override;
};
