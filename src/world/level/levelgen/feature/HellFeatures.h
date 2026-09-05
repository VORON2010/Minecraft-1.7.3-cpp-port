#pragma once

#include "world/level/levelgen/feature/Feature.h"


class FireFeature : public Feature
{
public:
	bool place(Level &level, Random &random, int_t x, int_t y, int_t z) override;
};


class GlowStone1Feature : public Feature
{
public:
	bool place(Level &level, Random &random, int_t x, int_t y, int_t z) override;
};


class GlowStone2Feature : public Feature
{
public:
	bool place(Level &level, Random &random, int_t x, int_t y, int_t z) override;
};


class HellLavaFeature : public Feature
{
	int_t liquidId;
public:
	HellLavaFeature(int_t liquidId);
	bool place(Level &level, Random &random, int_t x, int_t y, int_t z) override;
};
