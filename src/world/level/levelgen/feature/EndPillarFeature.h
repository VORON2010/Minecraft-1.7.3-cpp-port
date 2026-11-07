#pragma once

#include "world/level/levelgen/feature/Feature.h"

// 1.0 EndPillarFeature: obsidian column with a crystal on bedrock at the top
class EndPillarFeature : public Feature
{
	int_t tile;

public:
	explicit EndPillarFeature(int_t tile) : tile(tile) {}
	bool place(Level &level, Random &random, int_t x, int_t y, int_t z) override;
};
