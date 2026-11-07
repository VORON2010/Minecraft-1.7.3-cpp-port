#pragma once

#include <array>
#include <memory>
#include <unordered_map>
#include <vector>

#include "java/Random.h"
#include "java/Type.h"

class Level;
class Layer;
class Biome;

struct BiomeBlockPos
{
	int_t x = 0;
	int_t z = 0;
	bool found = false;
};

// 1.0 biome source: biomes come from the layer stack, cached per chunk.
class BiomeSource
{
private:
	struct CacheEntry
	{
		std::array<float, 256> temperatures = {};
		std::array<float, 256> downfalls = {};
		std::array<Biome *, 256> biomes = {};
		int_t chunkX = 0;
		int_t chunkZ = 0;
		long_t lastUse = 0;
	};

	std::shared_ptr<Layer> layer;
	std::shared_ptr<Layer> zoomedLayer;
	std::shared_ptr<Layer> temperatureLayer;
	std::shared_ptr<Layer> downfallLayer;

	std::unordered_map<long_t, std::unique_ptr<CacheEntry>> cache;
	long_t lastCleanup = 0;

	CacheEntry &getCacheEntry(int_t x, int_t z);

protected:
	BiomeSource();

public:
	std::vector<Biome *> playerSpawnBiomes;

	explicit BiomeSource(Level &level);
	virtual ~BiomeSource() {}

	virtual Biome *getBiome(int_t x, int_t z);
	virtual float getDownfall(int_t x, int_t z);
	virtual float getTemperature(int_t x, int_t y, int_t z);
	float getTemperature(int_t x, int_t z) { return getTemperature(x, 64, z); }
	virtual float scaleTemperature(float temperature, int_t y) { (void)y; return temperature; }

	virtual void getDownfallBlock(float *out, int_t x, int_t z, int_t w, int_t h);
	virtual void getTemperatureBlock(float *out, int_t x, int_t z, int_t w, int_t h);
	virtual void getRawBiomeBlock(std::vector<Biome *> &out, int_t x, int_t z, int_t w, int_t h);
	virtual void getBiomeBlock(std::vector<Biome *> &out, int_t x, int_t z, int_t w, int_t h, bool useCache = true);

	virtual bool containsOnly(int_t x, int_t z, int_t r, const std::vector<Biome *> &allowed);
	virtual BiomeBlockPos findBiome(int_t x, int_t z, int_t r, const std::vector<Biome *> &allowed, Random &random);

	virtual void update();
};

// Single-biome source (Nether, End)
class FixedBiomeSource : public BiomeSource
{
private:
	Biome *biome;
	float temperature;
	float downfall;

public:
	FixedBiomeSource(Biome *biome, float temperature, float downfall);

	Biome *getBiome(int_t x, int_t z) override;
	float getDownfall(int_t x, int_t z) override;
	float getTemperature(int_t x, int_t y, int_t z) override;
	void getDownfallBlock(float *out, int_t x, int_t z, int_t w, int_t h) override;
	void getTemperatureBlock(float *out, int_t x, int_t z, int_t w, int_t h) override;
	void getRawBiomeBlock(std::vector<Biome *> &out, int_t x, int_t z, int_t w, int_t h) override;
	void getBiomeBlock(std::vector<Biome *> &out, int_t x, int_t z, int_t w, int_t h, bool useCache = true) override;
	bool containsOnly(int_t x, int_t z, int_t r, const std::vector<Biome *> &allowed) override;
	BiomeBlockPos findBiome(int_t x, int_t z, int_t r, const std::vector<Biome *> &allowed, Random &random) override;
	void update() override {}
};
