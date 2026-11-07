#pragma once

#include "world/level/levelgen/synth/Synth.h"

#include <list>

#include "world/level/levelgen/synth/ImprovedNoise.h"

#include "java/Type.h"
#include "java/Random.h"

class PerlinNoise : public Synth
{
private:
	std::list<ImprovedNoise> noiseLevels;
	int_t levels = 0;

public:
	PerlinNoise(int_t levels);
	PerlinNoise(Random &random, int_t levels);
	PerlinNoise(Random &&random, int_t levels);

	double getValue(double x, double y) override;
	double getValue(double x, double y, double z);

	// 1.0: integer origin, wrapped at 2^24 so terrain stays precise far from 0,0
	void getRegion(double *out, int_t x, int_t y, int_t z, int_t xSize, int_t ySize, int_t zSize, double xScale, double yScale, double zScale);
	void getRegion(double *out, int_t x, int_t z, int_t xSize, int_t zSize, double xScale, double zScale, double unknown);
};
