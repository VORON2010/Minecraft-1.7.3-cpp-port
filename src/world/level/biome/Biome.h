#pragma once

#include <array>
#include <functional>
#include <memory>
#include <vector>

#include "java/Random.h"
#include "java/String.h"
#include "java/Type.h"

class Level;
class LevelSource;
class Mob;
class Feature;
class BiomeDecorator;

enum class MobCategory
{
	Monster,
	Creature,
	WaterCreature,
};

// Natural spawn entry: weight and group size like vanilla, plus a factory for the mob.
struct BiomeSpawnEntry
{
	std::function<std::shared_ptr<Mob>(Level &)> create;
	int_t weight = 1;
	int_t minCount = 1;
	int_t maxCount = 1;
};

// Biomes from release 1.0 (the b1.8 set plus mushroom islands and the frozen biomes).
class Biome
{
public:
	static std::array<Biome *, 256> biomes;

	static Biome *ocean;
	static Biome *plains;
	static Biome *desert;
	static Biome *extremeHills;
	static Biome *forest;
	static Biome *taiga;
	static Biome *swampland;
	static Biome *river;
	static Biome *hell;
	static Biome *sky;
	static Biome *frozenOcean;
	static Biome *frozenRiver;
	static Biome *icePlains;
	static Biome *iceMountains;
	static Biome *mushroomIsland;
	static Biome *mushroomIslandShore;

	static void init();

	const int_t id;
	jstring name;
	int_t color = 0;
	ubyte_t topMaterial;
	ubyte_t material;
	int_t leafColor = 5169201;
	float depth = 0.1f;
	float scale = 0.3f;
	float temperature = 0.5f;
	float downfall = 0.5f;
	int_t waterColor = 0xFFFFFF;
	std::unique_ptr<BiomeDecorator> decorator;

protected:
	std::vector<BiomeSpawnEntry> enemies;
	std::vector<BiomeSpawnEntry> friendlies;
	std::vector<BiomeSpawnEntry> waterFriendlies;
	bool snowCovered = false;
	bool rain = true;

	std::unique_ptr<Feature> normalTree;
	std::unique_ptr<Feature> fancyTree;
	std::unique_ptr<Feature> birchTree;
	std::unique_ptr<Feature> swampTree;

public:
	explicit Biome(int_t id);
	virtual ~Biome();

	Biome &setTemperatureAndDownfall(float temperature, float downfall);
	Biome &setDepthAndScale(float depth, float scale);
	Biome &setNoRain() { rain = false; return *this; }
	Biome &setName(const jstring &name) { this->name = name; return *this; }
	Biome &setLeafColor(int_t color) { leafColor = color; return *this; }
	Biome &setColor(int_t color) { this->color = color; return *this; }

	virtual Feature &getTreeFeature(Random &random);
	virtual int_t getSkyColor(float temperature);
	const std::vector<BiomeSpawnEntry> &getMobs(MobCategory category) const;

	bool hasSnow() const { return snowCovered; }
	bool hasRain() const { return snowCovered ? false : rain; }
	virtual float getCreatureProbability() { return 0.1f; }
	int_t getDownfallInt() const { return static_cast<int_t>(downfall * 65536.0f); }
	int_t getTemperatureInt() const { return static_cast<int_t>(temperature * 65536.0f); }

	virtual void decorate(Level &level, Random &random, int_t x, int_t z);
	virtual int_t getGrassColor(LevelSource &level, int_t x, int_t y, int_t z);
	virtual int_t getFolageColor(LevelSource &level, int_t x, int_t y, int_t z);
};
