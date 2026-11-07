#pragma once

#include <array>
#include <memory>

#include "java/Random.h"
#include "world/level/chunk/ChunkSource.h"
#include "world/level/levelgen/synth/PerlinNoise.h"

class Level;

// 1.0 TheEndChunkGenerator: one floating island of end stone around 0,0
class TheEndLevelSource : public ChunkSource
{
	Random random;
	PerlinNoise lperlinNoise1;
	PerlinNoise lperlinNoise2;
	PerlinNoise perlinNoise1;
	PerlinNoise scaleNoise;
	PerlinNoise depthNoise;
	Level &level;

	std::array<double, 3 * 33 * 3> heights = {};
	std::array<double, 3 * 33 * 3> pnr = {};
	std::array<double, 3 * 33 * 3> ar = {};
	std::array<double, 3 * 33 * 3> br = {};
	std::array<double, 3 * 3> sr = {};
	std::array<double, 3 * 3> dr = {};

public:
	TheEndLevelSource(Level &level, long_t seed);

	void prepareHeights(int_t x, int_t z, ubyte_t *tiles);
	std::shared_ptr<LevelChunk> getChunk(int_t x, int_t z) override;
	bool hasChunk(int_t x, int_t z) override;
	void postProcess(ChunkSource &parent, int_t x, int_t z) override;
	bool save(bool force, std::shared_ptr<ProgressListener> progressListener) override;
	bool tick() override;
	bool shouldSave() override;
	jstring gatherStats() override;

private:
	void getHeights(double *out, int_t x, int_t y, int_t z, int_t xd, int_t yd, int_t zd);
};
