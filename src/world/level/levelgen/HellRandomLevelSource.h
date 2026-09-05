#pragma once

#include "world/level/chunk/ChunkSource.h"

#include <array>
#include <vector>

#include "world/level/levelgen/HellCaveFeature.h"
#include "world/level/levelgen/synth/PerlinNoise.h"

#include "java/Random.h"

class Level;


class HellRandomLevelSource : public ChunkSource
{
private:
	Random random;

	PerlinNoise lperlinNoise1;
	PerlinNoise lperlinNoise2;
	PerlinNoise perlinNoise1;
	PerlinNoise perlinNoise2;
	PerlinNoise perlinNoise3;
	PerlinNoise scaleNoise;
	PerlinNoise depthNoise;

	Level &level;

	HellCaveFeature caveFeature;

	std::array<double, 5 * 17 * 5> buffer = {};
	std::array<double, 16 * 16> sandBuffer = {};
	std::array<double, 16 * 16> gravelBuffer = {};
	std::array<double, 16 * 16> depthBuffer = {};

	std::array<double, 5 * 17 * 5> pnr = {};
	std::array<double, 5 * 17 * 5> ar = {};
	std::array<double, 5 * 17 * 5> br = {};
	std::array<double, 5 * 5> sr = {};
	std::array<double, 5 * 5> dr = {};

public:
	HellRandomLevelSource(Level &level, long_t seed);

	void prepareHeights(int_t x, int_t z, ubyte_t *tiles);
	void buildSurfaces(int_t x, int_t z, ubyte_t *tiles);
	std::shared_ptr<LevelChunk> getChunk(int_t x, int_t z) override;
private:
	void getHeights(double *out, int_t x, int_t y, int_t z, int_t xd, int_t yd, int_t zd);
public:
	bool hasChunk(int_t x, int_t z) override;
	void postProcess(ChunkSource &parent, int_t x, int_t z) override;
	bool save(bool force, std::shared_ptr<ProgressListener> progressListener) override;
	bool tick() override;
	bool shouldSave() override;
	jstring gatherStats() override;
};
