#pragma once

#include "world/level/chunk/ChunkSource.h"

#include <array>
#include <memory>
#include <vector>

#include "world/level/levelgen/synth/PerlinNoise.h"
#include "world/level/levelgen/LargeCaveFeature.h"
#include "world/level/levelgen/RavineFeature.h"

#include "java/Random.h"

class Level;
class Biome;
class MineShaftFeature;
class VillageFeature;
class StrongholdFeature;

// Overworld chunk generator from release 1.0 (b1.8 terrain with biome heights, ravines, structures).
class RandomLevelSource : public ChunkSource
{
public:
	static constexpr int_t SEA_LEVEL = 63;

private:
	Random random;

	PerlinNoise lperlinNoise1;
	PerlinNoise lperlinNoise2;
	PerlinNoise perlinNoise1;
	PerlinNoise perlinNoise3;

public:
	PerlinNoise scaleNoise;
	PerlinNoise depthNoise;
	PerlinNoise forestNoise;

private:
	Level &level;
	bool generateStructures;
	LargeCaveFeature caveFeature;
	RavineFeature canyonFeature;

public:
	std::unique_ptr<StrongholdFeature> strongholdFeature;
	std::unique_ptr<VillageFeature> villageFeature;
	std::unique_ptr<MineShaftFeature> mineShaftFeature;

private:
	std::vector<Biome *> biomes;
	std::vector<double> buffer;
	std::array<double, 256> depthBuffer = {};
	std::vector<double> pnr, ar, br, sr, dr;
	std::array<float, 25> biomeWeights = {};
	bool biomeWeightsReady = false;

	void getHeights(std::vector<double> &out, int_t x, int_t y, int_t z, int_t xd, int_t yd, int_t zd);

public:
	RandomLevelSource(Level &level, long_t seed, bool generateStructures = true);
	~RandomLevelSource() override;

	void prepareHeights(int_t x, int_t z, ubyte_t *tiles);
	void buildSurfaces(int_t x, int_t z, ubyte_t *tiles, std::vector<Biome *> &biomes);

	std::shared_ptr<LevelChunk> getChunk(int_t x, int_t z) override;
	bool hasChunk(int_t x, int_t z) override;
	void postProcess(ChunkSource &parent, int_t x, int_t z) override;
	bool save(bool force, std::shared_ptr<ProgressListener> progressListener) override;
	bool tick() override;
	bool shouldSave() override;
	jstring gatherStats() override;

	// position of the nearest stronghold (for eyes of ender); returns false if none
	bool findNearestStronghold(int_t x, int_t y, int_t z, int_t &outX, int_t &outY, int_t &outZ) override;
};
