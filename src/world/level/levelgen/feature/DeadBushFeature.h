#pragma once

#include "world/level/levelgen/feature/Feature.h"

class DeadBushFeature : public Feature {
	int_t deadBushId;

public:
	DeadBushFeature(int_t deadBushId);
	bool place(Level &level, Random &random, int_t x, int_t y, int_t z) override;
};
