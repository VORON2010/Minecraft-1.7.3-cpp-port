#pragma once

#include <memory>

#include "java/Random.h"
#include "java/Type.h"

class Level;
class Biome;
class Feature;

// Per-biome population of a chunk (ores, trees, plants, springs). Ported from the 1.0 decorator.
class BiomeDecorator
{
protected:
	Level *level = nullptr;
	Random *random = nullptr;
	int_t xo = 0;
	int_t zo = 0;
	Biome &biome;

	std::unique_ptr<Feature> clayFeature;
	std::unique_ptr<Feature> sandFeature;
	std::unique_ptr<Feature> gravelFeature;
	std::unique_ptr<Feature> dirtOreFeature;
	std::unique_ptr<Feature> gravelOreFeature;
	std::unique_ptr<Feature> coalOreFeature;
	std::unique_ptr<Feature> ironOreFeature;
	std::unique_ptr<Feature> goldOreFeature;
	std::unique_ptr<Feature> redStoneOreFeature;
	std::unique_ptr<Feature> diamondOreFeature;
	std::unique_ptr<Feature> lapisOreFeature;
	std::unique_ptr<Feature> yellowFlowerFeature;
	std::unique_ptr<Feature> roseFlowerFeature;
	std::unique_ptr<Feature> brownMushroomFeature;
	std::unique_ptr<Feature> redMushroomFeature;
	std::unique_ptr<Feature> hugeMushroomFeature;
	std::unique_ptr<Feature> reedsFeature;
	std::unique_ptr<Feature> cactusFeature;
	std::unique_ptr<Feature> waterlilyFeature;

public:
	int_t waterlilyCount = 0;
	int_t treeCount = 0;
	int_t flowerCount = 2;
	int_t grassCount = 1;
	int_t deadBushCount = 0;
	int_t mushroomCount = 0;
	int_t reedsCount = 0;
	int_t cactusCount = 0;
	int_t gravelCount = 1;
	int_t sandCount = 3;
	int_t clayCount = 1;
	int_t hugeMushrooms = 0;
	bool liquids = true;

	explicit BiomeDecorator(Biome &biome);
	virtual ~BiomeDecorator();

	void decorate(Level &level, Random &random, int_t xo, int_t zo);

protected:
	virtual void decorate();
	void decorateDepthSpan(int_t count, Feature &feature, int_t y0, int_t y1);
	void decorateDepthAverage(int_t count, Feature &feature, int_t yMid, int_t ySpan);
	void decorateOres();
};
