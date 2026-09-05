#pragma once

#include <vector>

#include "world/level/levelgen/synth/PerlinSimplexNoise.h"

#include "java/Type.h"

class Level;

enum class TreeStyle
{
	Default,
	Rainforest,
	Forest,
	Taiga,
};

enum class BiomeId
{
	Rainforest,
	Swampland,
	SeasonalForest,
	Forest,
	Savanna,
	Shrubland,
	Taiga,
	Desert,
	Plains,
	Tundra,
	Hell,
};

struct BiomeInfo
{
	int_t topTileId = 0;
	int_t fillerTileId = 0;
	bool enableSnow = false;
	TreeStyle treeStyle = TreeStyle::Default;
	int_t yellowFlowerCount = 0;
	int_t tallGrassCount = 0;
	int_t deadBushCount = 0;
	int_t cactusCount = 0;
	bool prefersFern = false;
	bool enableRain = true;

	bool canSpawnLightningBolt() const { return !enableSnow && enableRain; }
};

class BiomeSource
{
private:
	PerlinSimplexNoise temperatureMap;
	PerlinSimplexNoise downfallMap;
	PerlinSimplexNoise noiseMap;

public:
	std::vector<double> temperatures = std::vector<double>(16 * 16);
	std::vector<double> downfalls = std::vector<double>(16 * 16);
	std::vector<double> noises = std::vector<double>(16 * 16);
	std::vector<BiomeId> biomes = std::vector<BiomeId>(16 * 16);

private:
	static constexpr float zoom = 2.0f;
	static constexpr float tempScale = 0.025f;
	static constexpr float downfallScale = 0.05f;
	static constexpr float noiseScale = 0.25f;

	static BiomeId pickBiome(float temperature, float downfall);
	static BiomeId getBiomeFromLookup(double temperature, double downfall);

public:
	BiomeSource(Level &level);
	virtual ~BiomeSource() {}

	virtual double getTemperature(int_t x, int_t z);
	virtual void getTemperatures(double *out, int_t x, int_t z, int_t xd, int_t zd);
	virtual BiomeId getBiome(int_t x, int_t z);
	const BiomeInfo &getBiomeInfo(BiomeId biome) const;

	virtual void getBiomeBlock(int_t x, int_t z, int_t xd, int_t zd);
};


class HellBiomeSource : public BiomeSource
{
public:
	HellBiomeSource(Level &level);

	double getTemperature(int_t x, int_t z) override;
	void getTemperatures(double *out, int_t x, int_t z, int_t xd, int_t zd) override;
	BiomeId getBiome(int_t x, int_t z) override;
	void getBiomeBlock(int_t x, int_t z, int_t xd, int_t zd) override;
};
