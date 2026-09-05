#pragma once

#include "world/level/levelgen/feature/Feature.h"

class TallGrassFeature : public Feature {
	int_t tallGrassId;
	int_t tallGrassData;

public:
	TallGrassFeature(int_t tallGrassId, int_t tallGrassData);
	bool place(Level &level, Random &random, int_t x, int_t y, int_t z) override;
};
