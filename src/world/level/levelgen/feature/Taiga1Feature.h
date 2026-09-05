#pragma once

#include "world/level/levelgen/feature/Feature.h"

class Taiga1Feature : public Feature {
public:
	bool place(Level &level, Random &random, int_t x, int_t y, int_t z) override;
};
