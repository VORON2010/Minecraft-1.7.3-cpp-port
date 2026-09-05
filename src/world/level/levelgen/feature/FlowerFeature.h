#pragma once

#include "world/level/levelgen/feature/Feature.h"

class FlowerFeature : public Feature {
	int_t flowerId;

public:
	FlowerFeature(int_t flowerId);
	bool place(Level &level, Random &random, int_t x, int_t y, int_t z) override;
};
