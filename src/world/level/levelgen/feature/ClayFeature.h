#pragma once

#include "world/level/levelgen/feature/Feature.h"

class ClayFeature : public Feature {
	int_t count;
public:
	ClayFeature(int_t count);
	bool place(Level &level, Random &random, int_t x, int_t y, int_t z) override;
};
