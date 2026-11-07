#pragma once

#include "world/level/levelgen/feature/Feature.h"

// b1.8 huge brown / red mushroom
class HugeMushroomFeature : public Feature
{
private:
	int_t forcedType = -1;

public:
	HugeMushroomFeature() = default;
	explicit HugeMushroomFeature(int_t forcedType);
	bool place(Level &level, Random &random, int_t x, int_t y, int_t z) override;
};
