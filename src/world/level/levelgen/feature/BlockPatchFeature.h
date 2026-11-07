#pragma once

#include "world/level/levelgen/feature/Feature.h"

// b1.8 sand / gravel patches under water
class BlockPatchFeature : public Feature
{
private:
	int_t block;
	int_t radius;

public:
	BlockPatchFeature(int_t radius, int_t block);
	bool place(Level &level, Random &random, int_t x, int_t y, int_t z) override;
};
