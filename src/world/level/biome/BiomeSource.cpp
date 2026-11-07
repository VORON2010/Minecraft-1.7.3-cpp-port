#include "world/level/biome/BiomeSource.h"

#include <algorithm>

#include "java/System.h"
#include "world/level/Level.h"
#include "world/level/biome/Biome.h"
#include "world/level/biome/layer/Layer.h"

BiomeSource::BiomeSource()
{
	Biome::init();
	playerSpawnBiomes = {Biome::forest, Biome::swampland, Biome::taiga};
}

BiomeSource::BiomeSource(Level &level) : BiomeSource()
{
	std::vector<std::shared_ptr<Layer>> layers = Layer::getDefaultLayers(level.seed);
	layer = layers[0];
	zoomedLayer = layers[1];
	temperatureLayer = layers[2];
	downfallLayer = layers[3];
}

BiomeSource::CacheEntry &BiomeSource::getCacheEntry(int_t x, int_t z)
{
	x >>= 4;
	z >>= 4;
	long_t key = static_cast<long_t>(static_cast<ulong_t>(static_cast<uint_t>(x)) | static_cast<ulong_t>(static_cast<uint_t>(z)) << 32);
	auto it = cache.find(key);
	if (it == cache.end())
	{
		auto entry = std::make_unique<CacheEntry>();
		entry->chunkX = x;
		entry->chunkZ = z;
		getTemperatureBlock(entry->temperatures.data(), x << 4, z << 4, 16, 16);
		getDownfallBlock(entry->downfalls.data(), x << 4, z << 4, 16, 16);
		std::vector<Biome *> biomes;
		getBiomeBlock(biomes, x << 4, z << 4, 16, 16, false);
		std::copy(biomes.begin(), biomes.begin() + 256, entry->biomes.begin());
		it = cache.emplace(key, std::move(entry)).first;
	}
	it->second->lastUse = System::currentTimeMillis();
	return *it->second;
}

Biome *BiomeSource::getBiome(int_t x, int_t z)
{
	return getCacheEntry(x, z).biomes[(x & 15) | (z & 15) << 4];
}

float BiomeSource::getDownfall(int_t x, int_t z)
{
	return getCacheEntry(x, z).downfalls[(x & 15) | (z & 15) << 4];
}

float BiomeSource::getTemperature(int_t x, int_t y, int_t z)
{
	return scaleTemperature(getCacheEntry(x, z).temperatures[(x & 15) | (z & 15) << 4], y);
}

void BiomeSource::getDownfallBlock(float *out, int_t x, int_t z, int_t w, int_t h)
{
	std::vector<int_t> values = downfallLayer->getArea(x, z, w, h);
	for (int_t i = 0; i < w * h; ++i)
		out[i] = std::min(1.0f, static_cast<float>(values[i]) / 65536.0f);
}

void BiomeSource::getTemperatureBlock(float *out, int_t x, int_t z, int_t w, int_t h)
{
	std::vector<int_t> values = temperatureLayer->getArea(x, z, w, h);
	for (int_t i = 0; i < w * h; ++i)
		out[i] = std::min(1.0f, static_cast<float>(values[i]) / 65536.0f);
}

void BiomeSource::getRawBiomeBlock(std::vector<Biome *> &out, int_t x, int_t z, int_t w, int_t h)
{
	std::vector<int_t> values = layer->getArea(x, z, w, h);
	out.resize(static_cast<size_t>(w * h));
	for (int_t i = 0; i < w * h; ++i)
		out[i] = Biome::biomes[values[i]];
}

void BiomeSource::getBiomeBlock(std::vector<Biome *> &out, int_t x, int_t z, int_t w, int_t h, bool useCache)
{
	out.resize(static_cast<size_t>(w * h));
	if (useCache && w == 16 && h == 16 && (x & 15) == 0 && (z & 15) == 0)
	{
		CacheEntry &entry = getCacheEntry(x, z);
		std::copy(entry.biomes.begin(), entry.biomes.end(), out.begin());
		return;
	}
	std::vector<int_t> values = zoomedLayer->getArea(x, z, w, h);
	for (int_t i = 0; i < w * h; ++i)
		out[i] = Biome::biomes[values[i]];
}

bool BiomeSource::containsOnly(int_t x, int_t z, int_t r, const std::vector<Biome *> &allowed)
{
	int_t x0 = (x - r) >> 2;
	int_t z0 = (z - r) >> 2;
	int_t x1 = (x + r) >> 2;
	int_t z1 = (z + r) >> 2;
	int_t w = x1 - x0 + 1;
	int_t h = z1 - z0 + 1;
	std::vector<int_t> values = layer->getArea(x0, z0, w, h);
	for (int_t i = 0; i < w * h; ++i)
	{
		Biome *b = Biome::biomes[values[i]];
		if (std::find(allowed.begin(), allowed.end(), b) == allowed.end())
			return false;
	}
	return true;
}

BiomeBlockPos BiomeSource::findBiome(int_t x, int_t z, int_t r, const std::vector<Biome *> &allowed, Random &random)
{
	int_t x0 = (x - r) >> 2;
	int_t z0 = (z - r) >> 2;
	int_t x1 = (x + r) >> 2;
	int_t z1 = (z + r) >> 2;
	int_t w = x1 - x0 + 1;
	int_t h = z1 - z0 + 1;
	std::vector<int_t> values = layer->getArea(x0, z0, w, h);
	BiomeBlockPos result;
	int_t found = 0;
	for (int_t i = 0; i < static_cast<int_t>(values.size()); ++i)
	{
		int_t bx = (x0 + i % w) << 2;
		int_t bz = (z0 + i / w) << 2;
		Biome *b = Biome::biomes[values[i]];
		if (std::find(allowed.begin(), allowed.end(), b) != allowed.end() && (!result.found || random.nextInt(found + 1) == 0))
		{
			result.x = bx;
			result.z = bz;
			result.found = true;
			found++;
		}
	}
	return result;
}

void BiomeSource::update()
{
	long_t now = System::currentTimeMillis();
	long_t elapsed = now - lastCleanup;
	if (elapsed <= 7500 && elapsed >= 0)
		return;
	lastCleanup = now;
	for (auto it = cache.begin(); it != cache.end();)
	{
		long_t age = now - it->second->lastUse;
		if (age > 30000 || age < 0)
			it = cache.erase(it);
		else
			++it;
	}
}

FixedBiomeSource::FixedBiomeSource(Biome *biome, float temperature, float downfall)
	: biome(biome), temperature(temperature), downfall(downfall)
{
}

Biome *FixedBiomeSource::getBiome(int_t x, int_t z)
{
	(void)x; (void)z;
	return biome;
}

float FixedBiomeSource::getDownfall(int_t x, int_t z)
{
	(void)x; (void)z;
	return downfall;
}

float FixedBiomeSource::getTemperature(int_t x, int_t y, int_t z)
{
	(void)x; (void)y; (void)z;
	return temperature;
}

void FixedBiomeSource::getDownfallBlock(float *out, int_t x, int_t z, int_t w, int_t h)
{
	(void)x; (void)z;
	std::fill(out, out + w * h, downfall);
}

void FixedBiomeSource::getTemperatureBlock(float *out, int_t x, int_t z, int_t w, int_t h)
{
	(void)x; (void)z;
	std::fill(out, out + w * h, temperature);
}

void FixedBiomeSource::getRawBiomeBlock(std::vector<Biome *> &out, int_t x, int_t z, int_t w, int_t h)
{
	(void)x; (void)z;
	out.assign(static_cast<size_t>(w * h), biome);
}

void FixedBiomeSource::getBiomeBlock(std::vector<Biome *> &out, int_t x, int_t z, int_t w, int_t h, bool useCache)
{
	(void)x; (void)z; (void)useCache;
	out.assign(static_cast<size_t>(w * h), biome);
}

bool FixedBiomeSource::containsOnly(int_t x, int_t z, int_t r, const std::vector<Biome *> &allowed)
{
	(void)x; (void)z; (void)r;
	return std::find(allowed.begin(), allowed.end(), biome) != allowed.end();
}

BiomeBlockPos FixedBiomeSource::findBiome(int_t x, int_t z, int_t r, const std::vector<Biome *> &allowed, Random &random)
{
	BiomeBlockPos result;
	if (std::find(allowed.begin(), allowed.end(), biome) != allowed.end())
	{
		result.x = x - r + random.nextInt(r * 2 + 1);
		result.z = z - r + random.nextInt(r * 2 + 1);
		result.found = true;
	}
	return result;
}
