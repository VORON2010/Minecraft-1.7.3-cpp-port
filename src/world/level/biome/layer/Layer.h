#pragma once

#include <memory>
#include <vector>

#include "java/Type.h"

// The b1.8 / 1.0 biome layer stack (GenLayer). Each layer turns a parent int grid into a new one.
class Layer
{
private:
	long_t seed = 0;
	long_t worldSeed = 0;
	long_t rval = 0;

protected:
	std::shared_ptr<Layer> parent;

	void initRandom(long_t x, long_t z);
	int_t nextRandom(int_t bound);

public:
	explicit Layer(long_t seedMixup);
	virtual ~Layer() {}

	virtual void init(long_t worldSeed);
	virtual std::vector<int_t> getArea(int_t xo, int_t zo, int_t w, int_t h) = 0;

	// returns {noise biomes (1:4), biomes (1:1), temperatures, downfalls}
	static std::vector<std::shared_ptr<Layer>> getDefaultLayers(long_t seed);
};

class IslandLayer : public Layer
{
public:
	explicit IslandLayer(long_t seed) : Layer(seed) {}
	std::vector<int_t> getArea(int_t xo, int_t zo, int_t w, int_t h) override;
};

class FuzzyZoomLayer : public Layer
{
public:
	FuzzyZoomLayer(long_t seed, std::shared_ptr<Layer> parent) : Layer(seed) { this->parent = parent; }
	std::vector<int_t> getArea(int_t xo, int_t zo, int_t w, int_t h) override;

protected:
	int_t random(int_t a, int_t b) { return nextRandom(2) == 0 ? a : b; }
	int_t random(int_t a, int_t b, int_t c, int_t d);
};

class ZoomLayer : public Layer
{
public:
	ZoomLayer(long_t seed, std::shared_ptr<Layer> parent) : Layer(seed) { this->parent = parent; }
	std::vector<int_t> getArea(int_t xo, int_t zo, int_t w, int_t h) override;
	static std::shared_ptr<Layer> zoom(long_t seed, std::shared_ptr<Layer> layer, int_t count);

protected:
	int_t random(int_t a, int_t b) { return nextRandom(2) == 0 ? a : b; }
	int_t modeOrRandom(int_t a, int_t b, int_t c, int_t d);
};

class AddIslandLayer : public Layer
{
public:
	AddIslandLayer(long_t seed, std::shared_ptr<Layer> parent) : Layer(seed) { this->parent = parent; }
	std::vector<int_t> getArea(int_t xo, int_t zo, int_t w, int_t h) override;
};

class AddSnowLayer : public Layer
{
public:
	AddSnowLayer(long_t seed, std::shared_ptr<Layer> parent) : Layer(seed) { this->parent = parent; }
	std::vector<int_t> getArea(int_t xo, int_t zo, int_t w, int_t h) override;
};

class AddMushroomIslandLayer : public Layer
{
public:
	AddMushroomIslandLayer(long_t seed, std::shared_ptr<Layer> parent) : Layer(seed) { this->parent = parent; }
	std::vector<int_t> getArea(int_t xo, int_t zo, int_t w, int_t h) override;
};

class RiverInitLayer : public Layer
{
public:
	RiverInitLayer(long_t seed, std::shared_ptr<Layer> parent) : Layer(seed) { this->parent = parent; }
	std::vector<int_t> getArea(int_t xo, int_t zo, int_t w, int_t h) override;
};

class RiverLayer : public Layer
{
public:
	RiverLayer(long_t seed, std::shared_ptr<Layer> parent) : Layer(seed) { this->parent = parent; }
	std::vector<int_t> getArea(int_t xo, int_t zo, int_t w, int_t h) override;
};

class SmoothLayer : public Layer
{
public:
	SmoothLayer(long_t seed, std::shared_ptr<Layer> parent) : Layer(seed) { this->parent = parent; }
	std::vector<int_t> getArea(int_t xo, int_t zo, int_t w, int_t h) override;
};

class BiomeInitLayer : public Layer
{
public:
	BiomeInitLayer(long_t seed, std::shared_ptr<Layer> parent) : Layer(seed) { this->parent = parent; }
	std::vector<int_t> getArea(int_t xo, int_t zo, int_t w, int_t h) override;
};

class ShoreLayer : public Layer
{
public:
	ShoreLayer(long_t seed, std::shared_ptr<Layer> parent) : Layer(seed) { this->parent = parent; }
	std::vector<int_t> getArea(int_t xo, int_t zo, int_t w, int_t h) override;
};

class TemperatureLayer : public Layer
{
public:
	explicit TemperatureLayer(std::shared_ptr<Layer> parent) : Layer(0) { this->parent = parent; }
	std::vector<int_t> getArea(int_t xo, int_t zo, int_t w, int_t h) override;
};

class DownfallLayer : public Layer
{
public:
	explicit DownfallLayer(std::shared_ptr<Layer> parent) : Layer(0) { this->parent = parent; }
	std::vector<int_t> getArea(int_t xo, int_t zo, int_t w, int_t h) override;
};

class SmoothZoomLayer : public Layer
{
public:
	SmoothZoomLayer(long_t seed, std::shared_ptr<Layer> parent) : Layer(seed) { this->parent = parent; }
	std::vector<int_t> getArea(int_t xo, int_t zo, int_t w, int_t h) override;
	static std::shared_ptr<Layer> zoom(long_t seed, std::shared_ptr<Layer> layer, int_t count);
};

class TemperatureMixerLayer : public Layer
{
private:
	std::shared_ptr<Layer> temp;
	int_t layer;

public:
	TemperatureMixerLayer(std::shared_ptr<Layer> temp, std::shared_ptr<Layer> parent, int_t layer) : Layer(0), temp(temp), layer(layer) { this->parent = parent; }
	std::vector<int_t> getArea(int_t xo, int_t zo, int_t w, int_t h) override;
};

class DownfallMixerLayer : public Layer
{
private:
	std::shared_ptr<Layer> downfall;
	int_t layer;

public:
	DownfallMixerLayer(std::shared_ptr<Layer> downfall, std::shared_ptr<Layer> parent, int_t layer) : Layer(0), downfall(downfall), layer(layer) { this->parent = parent; }
	std::vector<int_t> getArea(int_t xo, int_t zo, int_t w, int_t h) override;
};

class RiverMixerLayer : public Layer
{
private:
	std::shared_ptr<Layer> biomes;
	std::shared_ptr<Layer> rivers;

public:
	RiverMixerLayer(long_t seed, std::shared_ptr<Layer> biomes, std::shared_ptr<Layer> rivers) : Layer(seed), biomes(biomes), rivers(rivers) {}
	void init(long_t worldSeed) override;
	std::vector<int_t> getArea(int_t xo, int_t zo, int_t w, int_t h) override;
};

class VoronoiZoomLayer : public Layer
{
public:
	VoronoiZoomLayer(long_t seed, std::shared_ptr<Layer> parent) : Layer(seed) { this->parent = parent; }
	std::vector<int_t> getArea(int_t xo, int_t zo, int_t w, int_t h) override;
};
