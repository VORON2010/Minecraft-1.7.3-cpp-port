#pragma once

#include "world/level/levelgen/feature/Feature.h"

// b1.8 swamp oak: wide leaves and hanging vines, can grow in shallow water
class SwampTreeFeature : public Feature
{
public:
	bool place(Level &level, Random &random, int_t x, int_t y, int_t z) override;

private:
	void addVine(Level &level, int_t x, int_t y, int_t z, int_t data);
};
