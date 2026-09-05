#pragma once

#include "world/level/levelgen/feature/Feature.h"
#include "java/Type.h"

class SpringFeature : public Feature {
	int_t blockId;
public:
	SpringFeature(int_t blockId);
	bool place(Level &level, Random &random, int_t x, int_t y, int_t z) override;
};
