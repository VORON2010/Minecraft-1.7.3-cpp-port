#include "world/level/biome/layer/Layer.h"

#include <algorithm>

#include "world/level/biome/Biome.h"

namespace
{
	// Java long arithmetic wraps; do it in unsigned to stay defined in C++.
	inline long_t mix(long_t s)
	{
		ulong_t u = static_cast<ulong_t>(s);
		return static_cast<long_t>(u * (u * 6364136223846793005ULL + 1442695040888963407ULL));
	}

	inline long_t add(long_t a, long_t b)
	{
		return static_cast<long_t>(static_cast<ulong_t>(a) + static_cast<ulong_t>(b));
	}

	void copyRows(const std::vector<int_t> &src, int_t srcStride, int_t xo, int_t zo, int_t w, int_t h, std::vector<int_t> &out, int_t mask)
	{
		for (int_t z = 0; z < h; ++z)
		{
			int_t srcPos = (z + (zo & mask)) * srcStride + (xo & mask);
			std::copy(src.begin() + srcPos, src.begin() + srcPos + w, out.begin() + z * w);
		}
	}
}

Layer::Layer(long_t seedMixup)
{
	seed = seedMixup;
	seed = add(mix(seed), seedMixup);
	seed = add(mix(seed), seedMixup);
	seed = add(mix(seed), seedMixup);
}

void Layer::init(long_t worldSeed)
{
	this->worldSeed = worldSeed;
	if (parent != nullptr)
		parent->init(worldSeed);
	this->worldSeed = add(mix(this->worldSeed), seed);
	this->worldSeed = add(mix(this->worldSeed), seed);
	this->worldSeed = add(mix(this->worldSeed), seed);
}

void Layer::initRandom(long_t x, long_t z)
{
	rval = worldSeed;
	rval = add(mix(rval), x);
	rval = add(mix(rval), z);
	rval = add(mix(rval), x);
	rval = add(mix(rval), z);
}

int_t Layer::nextRandom(int_t bound)
{
	int_t result = static_cast<int_t>((rval >> 24) % static_cast<long_t>(bound));
	if (result < 0)
		result += bound;
	rval = add(mix(rval), worldSeed);
	return result;
}

std::vector<std::shared_ptr<Layer>> Layer::getDefaultLayers(long_t seed)
{
	Biome::init();

	std::shared_ptr<Layer> islands = std::make_shared<IslandLayer>(1);
	islands = std::make_shared<FuzzyZoomLayer>(2000, islands);
	islands = std::make_shared<AddIslandLayer>(1, islands);
	islands = std::make_shared<ZoomLayer>(2001, islands);
	islands = std::make_shared<AddIslandLayer>(2, islands);
	islands = std::make_shared<AddSnowLayer>(2, islands);
	islands = std::make_shared<ZoomLayer>(2002, islands);
	islands = std::make_shared<AddIslandLayer>(3, islands);
	islands = std::make_shared<ZoomLayer>(2003, islands);
	islands = std::make_shared<AddIslandLayer>(4, islands);
	islands = std::make_shared<AddMushroomIslandLayer>(5, islands);

	const int_t zoomLevel = 4;

	std::shared_ptr<Layer> riverLayer = ZoomLayer::zoom(1000, islands, 0);
	riverLayer = std::make_shared<RiverInitLayer>(100, riverLayer);
	riverLayer = ZoomLayer::zoom(1000, riverLayer, zoomLevel + 2);
	riverLayer = std::make_shared<RiverLayer>(1, riverLayer);
	riverLayer = std::make_shared<SmoothLayer>(1000, riverLayer);

	std::shared_ptr<Layer> biomeLayer = ZoomLayer::zoom(1000, islands, 0);
	biomeLayer = std::make_shared<BiomeInitLayer>(200, biomeLayer);
	biomeLayer = ZoomLayer::zoom(1000, biomeLayer, 2);

	std::shared_ptr<Layer> temperatureLayer = std::make_shared<TemperatureLayer>(biomeLayer);
	std::shared_ptr<Layer> downfallLayer = std::make_shared<DownfallLayer>(biomeLayer);

	for (int_t i = 0; i < zoomLevel; ++i)
	{
		biomeLayer = std::make_shared<ZoomLayer>(1000 + i, biomeLayer);
		if (i == 0)
			biomeLayer = std::make_shared<AddIslandLayer>(3, biomeLayer);
		if (i == 0)
			biomeLayer = std::make_shared<ShoreLayer>(1000, biomeLayer);
		temperatureLayer = std::make_shared<SmoothZoomLayer>(1000 + i, temperatureLayer);
		temperatureLayer = std::make_shared<TemperatureMixerLayer>(temperatureLayer, biomeLayer, i);
		downfallLayer = std::make_shared<SmoothZoomLayer>(1000 + i, downfallLayer);
		downfallLayer = std::make_shared<DownfallMixerLayer>(downfallLayer, biomeLayer, i);
	}

	biomeLayer = std::make_shared<SmoothLayer>(1000, biomeLayer);
	std::shared_ptr<Layer> riverMixer = std::make_shared<RiverMixerLayer>(100, biomeLayer, riverLayer);
	temperatureLayer = SmoothZoomLayer::zoom(1000, temperatureLayer, 2);
	downfallLayer = SmoothZoomLayer::zoom(1000, downfallLayer, 2);
	std::shared_ptr<Layer> zoomed = std::make_shared<VoronoiZoomLayer>(10, riverMixer);

	riverMixer->init(seed);
	temperatureLayer->init(seed);
	downfallLayer->init(seed);
	zoomed->init(seed);

	return {riverMixer, zoomed, temperatureLayer, downfallLayer};
}

std::vector<int_t> IslandLayer::getArea(int_t xo, int_t zo, int_t w, int_t h)
{
	std::vector<int_t> result(static_cast<size_t>(w * h));
	for (int_t z = 0; z < h; ++z)
	{
		for (int_t x = 0; x < w; ++x)
		{
			initRandom(xo + x, zo + z);
			result[x + z * w] = nextRandom(10) == 0 ? 1 : 0;
		}
	}
	if (xo > -w && xo <= 0 && zo > -h && zo <= 0)
		result[-xo + -zo * w] = 1;
	return result;
}

int_t FuzzyZoomLayer::random(int_t a, int_t b, int_t c, int_t d)
{
	int_t s = nextRandom(4);
	if (s == 0) return a;
	if (s == 1) return b;
	return s == 2 ? c : d;
}

std::vector<int_t> FuzzyZoomLayer::getArea(int_t xo, int_t zo, int_t w, int_t h)
{
	int_t px = xo >> 1;
	int_t pz = zo >> 1;
	int_t pw = (w >> 1) + 3;
	int_t ph = (h >> 1) + 3;
	std::vector<int_t> p = parent->getArea(px, pz, pw, ph);
	std::vector<int_t> tmp(static_cast<size_t>(pw * 2 * ph * 2));
	int_t ww = pw << 1;
	for (int_t z = 0; z < ph - 1; ++z)
	{
		int_t row = (z << 1) * ww;
		int_t ul = p[0 + (z + 0) * pw];
		int_t dl = p[0 + (z + 1) * pw];
		for (int_t x = 0; x < pw - 1; ++x)
		{
			initRandom((x + px) * 2, (z + pz) * 2);
			int_t ur = p[x + 1 + (z + 0) * pw];
			int_t dr = p[x + 1 + (z + 1) * pw];
			tmp[row] = ul;
			tmp[row++ + ww] = random(ul, dl);
			tmp[row] = random(ul, ur);
			tmp[row++ + ww] = random(ul, ur, dl, dr);
			ul = ur;
			dl = dr;
		}
	}
	std::vector<int_t> result(static_cast<size_t>(w * h));
	copyRows(tmp, pw << 1, xo, zo, w, h, result, 1);
	return result;
}

int_t ZoomLayer::modeOrRandom(int_t a, int_t b, int_t c, int_t d)
{
	if (b == c && c == d) return b;
	if (a == b && a == c) return a;
	if (a == b && a == d) return a;
	if (a == c && a == d) return a;
	if (a == b && c != d) return a;
	if (a == c && b != d) return a;
	if (a == d && b != c) return a;
	if (b == a && c != d) return b;
	if (b == c && a != d) return b;
	if (b == d && a != c) return b;
	if (c == a && b != d) return c;
	if (c == b && a != d) return c;
	if (c == d && a != b) return c;
	if (d == a && b != c) return c;
	if (d == b && a != c) return c;
	if (d == c && a != b) return c;
	int_t s = nextRandom(4);
	if (s == 0) return a;
	if (s == 1) return b;
	return s == 2 ? c : d;
}

std::vector<int_t> ZoomLayer::getArea(int_t xo, int_t zo, int_t w, int_t h)
{
	int_t px = xo >> 1;
	int_t pz = zo >> 1;
	int_t pw = (w >> 1) + 3;
	int_t ph = (h >> 1) + 3;
	std::vector<int_t> p = parent->getArea(px, pz, pw, ph);
	std::vector<int_t> tmp(static_cast<size_t>(pw * 2 * ph * 2));
	int_t ww = pw << 1;
	for (int_t z = 0; z < ph - 1; ++z)
	{
		int_t row = (z << 1) * ww;
		int_t ul = p[0 + (z + 0) * pw];
		int_t dl = p[0 + (z + 1) * pw];
		for (int_t x = 0; x < pw - 1; ++x)
		{
			initRandom((x + px) * 2, (z + pz) * 2);
			int_t ur = p[x + 1 + (z + 0) * pw];
			int_t dr = p[x + 1 + (z + 1) * pw];
			tmp[row] = ul;
			tmp[row++ + ww] = random(ul, dl);
			tmp[row] = random(ul, ur);
			tmp[row++ + ww] = modeOrRandom(ul, ur, dl, dr);
			ul = ur;
			dl = dr;
		}
	}
	std::vector<int_t> result(static_cast<size_t>(w * h));
	copyRows(tmp, pw << 1, xo, zo, w, h, result, 1);
	return result;
}

std::shared_ptr<Layer> ZoomLayer::zoom(long_t seed, std::shared_ptr<Layer> layer, int_t count)
{
	std::shared_ptr<Layer> result = layer;
	for (int_t i = 0; i < count; ++i)
		result = std::make_shared<ZoomLayer>(seed + i, result);
	return result;
}

std::vector<int_t> AddIslandLayer::getArea(int_t xo, int_t zo, int_t w, int_t h)
{
	int_t px = xo - 1;
	int_t pz = zo - 1;
	int_t pw = w + 2;
	int_t ph = h + 2;
	std::vector<int_t> p = parent->getArea(px, pz, pw, ph);
	std::vector<int_t> result(static_cast<size_t>(w * h));
	const int_t icePlains = Biome::icePlains->id;
	const int_t frozenOcean = Biome::frozenOcean->id;
	for (int_t z = 0; z < h; ++z)
	{
		for (int_t x = 0; x < w; ++x)
		{
			int_t n1 = p[x + 0 + (z + 0) * pw];
			int_t n2 = p[x + 2 + (z + 0) * pw];
			int_t n3 = p[x + 0 + (z + 2) * pw];
			int_t n4 = p[x + 2 + (z + 2) * pw];
			int_t c = p[x + 1 + (z + 1) * pw];
			initRandom(x + xo, z + zo);
			if (c != 0 || (n1 == 0 && n2 == 0 && n3 == 0 && n4 == 0))
			{
				if (c > 0 && (n1 == 0 || n2 == 0 || n3 == 0 || n4 == 0))
				{
					if (nextRandom(5) == 0)
						result[x + z * w] = c == icePlains ? frozenOcean : 0;
					else
						result[x + z * w] = c;
				}
				else
				{
					result[x + z * w] = c;
				}
			}
			else
			{
				int_t max = 1;
				int_t value = 1;
				if (n1 != 0 && nextRandom(max++) == 0) value = n1;
				if (n2 != 0 && nextRandom(max++) == 0) value = n2;
				if (n3 != 0 && nextRandom(max++) == 0) value = n3;
				if (n4 != 0 && nextRandom(max++) == 0) value = n4;
				if (nextRandom(3) == 0)
					result[x + z * w] = value;
				else if (value == icePlains)
					result[x + z * w] = frozenOcean;
				else
					result[x + z * w] = 0;
			}
		}
	}
	return result;
}

std::vector<int_t> AddSnowLayer::getArea(int_t xo, int_t zo, int_t w, int_t h)
{
	int_t pw = w + 2;
	std::vector<int_t> p = parent->getArea(xo - 1, zo - 1, pw, h + 2);
	std::vector<int_t> result(static_cast<size_t>(w * h));
	for (int_t z = 0; z < h; ++z)
	{
		for (int_t x = 0; x < w; ++x)
		{
			int_t c = p[x + 1 + (z + 1) * pw];
			initRandom(x + xo, z + zo);
			if (c == 0)
			{
				result[x + z * w] = 0;
			}
			else
			{
				int_t r = nextRandom(5);
				result[x + z * w] = r == 0 ? Biome::icePlains->id : 1;
			}
		}
	}
	return result;
}

std::vector<int_t> AddMushroomIslandLayer::getArea(int_t xo, int_t zo, int_t w, int_t h)
{
	int_t pw = w + 2;
	std::vector<int_t> p = parent->getArea(xo - 1, zo - 1, pw, h + 2);
	std::vector<int_t> result(static_cast<size_t>(w * h));
	for (int_t z = 0; z < h; ++z)
	{
		for (int_t x = 0; x < w; ++x)
		{
			int_t n1 = p[x + 0 + (z + 0) * pw];
			int_t n2 = p[x + 2 + (z + 0) * pw];
			int_t n3 = p[x + 0 + (z + 2) * pw];
			int_t n4 = p[x + 2 + (z + 2) * pw];
			int_t c = p[x + 1 + (z + 1) * pw];
			initRandom(x + xo, z + zo);
			if (c == 0 && n1 == 0 && n2 == 0 && n3 == 0 && n4 == 0 && nextRandom(100) == 0)
				result[x + z * w] = Biome::mushroomIsland->id;
			else
				result[x + z * w] = c;
		}
	}
	return result;
}

std::vector<int_t> RiverInitLayer::getArea(int_t xo, int_t zo, int_t w, int_t h)
{
	std::vector<int_t> p = parent->getArea(xo, zo, w, h);
	std::vector<int_t> result(static_cast<size_t>(w * h));
	for (int_t z = 0; z < h; ++z)
	{
		for (int_t x = 0; x < w; ++x)
		{
			initRandom(x + xo, z + zo);
			result[x + z * w] = p[x + z * w] > 0 ? nextRandom(2) + 2 : 0;
		}
	}
	return result;
}

std::vector<int_t> RiverLayer::getArea(int_t xo, int_t zo, int_t w, int_t h)
{
	int_t pw = w + 2;
	std::vector<int_t> p = parent->getArea(xo - 1, zo - 1, pw, h + 2);
	std::vector<int_t> result(static_cast<size_t>(w * h));
	const int_t river = Biome::river->id;
	for (int_t z = 0; z < h; ++z)
	{
		for (int_t x = 0; x < w; ++x)
		{
			int_t l = p[x + 0 + (z + 1) * pw];
			int_t r = p[x + 2 + (z + 1) * pw];
			int_t u = p[x + 1 + (z + 0) * pw];
			int_t d = p[x + 1 + (z + 2) * pw];
			int_t c = p[x + 1 + (z + 1) * pw];
			if (c == 0 || l == 0 || r == 0 || u == 0 || d == 0)
				result[x + z * w] = river;
			else if (c == l && c == u && c == r && c == d)
				result[x + z * w] = -1;
			else
				result[x + z * w] = river;
		}
	}
	return result;
}

std::vector<int_t> SmoothLayer::getArea(int_t xo, int_t zo, int_t w, int_t h)
{
	int_t pw = w + 2;
	std::vector<int_t> p = parent->getArea(xo - 1, zo - 1, pw, h + 2);
	std::vector<int_t> result(static_cast<size_t>(w * h));
	for (int_t z = 0; z < h; ++z)
	{
		for (int_t x = 0; x < w; ++x)
		{
			int_t l = p[x + 0 + (z + 1) * pw];
			int_t r = p[x + 2 + (z + 1) * pw];
			int_t u = p[x + 1 + (z + 0) * pw];
			int_t d = p[x + 1 + (z + 2) * pw];
			int_t c = p[x + 1 + (z + 1) * pw];
			if (l == r && u == d)
			{
				initRandom(x + xo, z + zo);
				c = nextRandom(2) == 0 ? l : u;
			}
			else
			{
				if (l == r)
					c = l;
				if (u == d)
					c = u;
			}
			result[x + z * w] = c;
		}
	}
	return result;
}

std::vector<int_t> BiomeInitLayer::getArea(int_t xo, int_t zo, int_t w, int_t h)
{
	const Biome *start[] = {Biome::desert, Biome::forest, Biome::extremeHills, Biome::swampland, Biome::plains, Biome::taiga};
	std::vector<int_t> p = parent->getArea(xo, zo, w, h);
	std::vector<int_t> result(static_cast<size_t>(w * h));
	for (int_t z = 0; z < h; ++z)
	{
		for (int_t x = 0; x < w; ++x)
		{
			initRandom(x + xo, z + zo);
			int_t c = p[x + z * w];
			if (c == 0)
				result[x + z * w] = 0;
			else if (c == Biome::mushroomIsland->id)
				result[x + z * w] = c;
			else if (c == 1)
				result[x + z * w] = start[nextRandom(6)]->id;
			else
				result[x + z * w] = Biome::icePlains->id;
		}
	}
	return result;
}

std::vector<int_t> ShoreLayer::getArea(int_t xo, int_t zo, int_t w, int_t h)
{
	int_t pw = w + 2;
	std::vector<int_t> p = parent->getArea(xo - 1, zo - 1, pw, h + 2);
	std::vector<int_t> result(static_cast<size_t>(w * h));
	const int_t ocean = Biome::ocean->id;
	for (int_t z = 0; z < h; ++z)
	{
		for (int_t x = 0; x < w; ++x)
		{
			initRandom(x + xo, z + zo);
			int_t c = p[x + 1 + (z + 1) * pw];
			if (c == Biome::mushroomIsland->id)
			{
				int_t u = p[x + 1 + (z + 1 - 1) * pw];
				int_t r = p[x + 1 + 1 + (z + 1) * pw];
				int_t l = p[x + 1 - 1 + (z + 1) * pw];
				int_t d = p[x + 1 + (z + 1 + 1) * pw];
				if (u != ocean && r != ocean && l != ocean && d != ocean)
					result[x + z * w] = c;
				else
					result[x + z * w] = Biome::mushroomIslandShore->id;
			}
			else
			{
				result[x + z * w] = c;
			}
		}
	}
	return result;
}

std::vector<int_t> TemperatureLayer::getArea(int_t xo, int_t zo, int_t w, int_t h)
{
	std::vector<int_t> p = parent->getArea(xo, zo, w, h);
	std::vector<int_t> result(static_cast<size_t>(w * h));
	for (int_t i = 0; i < w * h; ++i)
		result[i] = Biome::biomes[p[i]]->getTemperatureInt();
	return result;
}

std::vector<int_t> DownfallLayer::getArea(int_t xo, int_t zo, int_t w, int_t h)
{
	std::vector<int_t> p = parent->getArea(xo, zo, w, h);
	std::vector<int_t> result(static_cast<size_t>(w * h));
	for (int_t i = 0; i < w * h; ++i)
		result[i] = Biome::biomes[p[i]]->getDownfallInt();
	return result;
}

std::vector<int_t> SmoothZoomLayer::getArea(int_t xo, int_t zo, int_t w, int_t h)
{
	int_t px = xo >> 1;
	int_t pz = zo >> 1;
	int_t pw = (w >> 1) + 3;
	int_t ph = (h >> 1) + 3;
	std::vector<int_t> p = parent->getArea(px, pz, pw, ph);
	std::vector<int_t> tmp(static_cast<size_t>(pw * 2 * ph * 2));
	int_t ww = pw << 1;
	for (int_t z = 0; z < ph - 1; ++z)
	{
		int_t row = (z << 1) * ww;
		int_t ul = p[0 + (z + 0) * pw];
		int_t dl = p[0 + (z + 1) * pw];
		for (int_t x = 0; x < pw - 1; ++x)
		{
			initRandom((x + px) * 2, (z + pz) * 2);
			int_t ur = p[x + 1 + (z + 0) * pw];
			int_t dr = p[x + 1 + (z + 1) * pw];
			tmp[row] = ul;
			tmp[row++ + ww] = ul + (dl - ul) * nextRandom(256) / 256;
			tmp[row] = ul + (ur - ul) * nextRandom(256) / 256;
			int_t top = ul + (ur - ul) * nextRandom(256) / 256;
			int_t bottom = dl + (dr - dl) * nextRandom(256) / 256;
			tmp[row++ + ww] = top + (bottom - top) * nextRandom(256) / 256;
			ul = ur;
			dl = dr;
		}
	}
	std::vector<int_t> result(static_cast<size_t>(w * h));
	copyRows(tmp, pw << 1, xo, zo, w, h, result, 1);
	return result;
}

std::shared_ptr<Layer> SmoothZoomLayer::zoom(long_t seed, std::shared_ptr<Layer> layer, int_t count)
{
	std::shared_ptr<Layer> result = layer;
	for (int_t i = 0; i < count; ++i)
		result = std::make_shared<SmoothZoomLayer>(seed + i, result);
	return result;
}

std::vector<int_t> TemperatureMixerLayer::getArea(int_t xo, int_t zo, int_t w, int_t h)
{
	std::vector<int_t> b = parent->getArea(xo, zo, w, h);
	std::vector<int_t> t = temp->getArea(xo, zo, w, h);
	std::vector<int_t> result(static_cast<size_t>(w * h));
	for (int_t i = 0; i < w * h; ++i)
		result[i] = t[i] + (Biome::biomes[b[i]]->getTemperatureInt() - t[i]) / (layer * 2 + 1);
	return result;
}

std::vector<int_t> DownfallMixerLayer::getArea(int_t xo, int_t zo, int_t w, int_t h)
{
	std::vector<int_t> b = parent->getArea(xo, zo, w, h);
	std::vector<int_t> d = downfall->getArea(xo, zo, w, h);
	std::vector<int_t> result(static_cast<size_t>(w * h));
	for (int_t i = 0; i < w * h; ++i)
		result[i] = d[i] + (Biome::biomes[b[i]]->getDownfallInt() - d[i]) / (layer + 1);
	return result;
}

void RiverMixerLayer::init(long_t worldSeed)
{
	biomes->init(worldSeed);
	rivers->init(worldSeed);
	Layer::init(worldSeed);
}

std::vector<int_t> RiverMixerLayer::getArea(int_t xo, int_t zo, int_t w, int_t h)
{
	std::vector<int_t> b = biomes->getArea(xo, zo, w, h);
	std::vector<int_t> r = rivers->getArea(xo, zo, w, h);
	std::vector<int_t> result(static_cast<size_t>(w * h));
	for (int_t i = 0; i < w * h; ++i)
	{
		if (b[i] == Biome::ocean->id)
			result[i] = b[i];
		else if (r[i] >= 0)
		{
			if (b[i] == Biome::icePlains->id)
				result[i] = Biome::frozenRiver->id;
			else if (b[i] != Biome::mushroomIsland->id && b[i] != Biome::mushroomIslandShore->id)
				result[i] = r[i];
			else
				result[i] = Biome::mushroomIslandShore->id;
		}
		else
			result[i] = b[i];
	}
	return result;
}

std::vector<int_t> VoronoiZoomLayer::getArea(int_t xo, int_t zo, int_t w, int_t h)
{
	xo -= 2;
	zo -= 2;
	const int_t bits = 2;
	const int_t ss = 1 << bits;
	int_t px = xo >> bits;
	int_t pz = zo >> bits;
	int_t pw = (w >> bits) + 3;
	int_t ph = (h >> bits) + 3;
	std::vector<int_t> p = parent->getArea(px, pz, pw, ph);
	int_t ww = pw << bits;
	int_t hh = ph << bits;
	std::vector<int_t> tmp(static_cast<size_t>(ww * hh));
	for (int_t z = 0; z < ph - 1; ++z)
	{
		int_t ul = p[0 + (z + 0) * pw];
		int_t dl = p[0 + (z + 1) * pw];
		for (int_t x = 0; x < pw - 1; ++x)
		{
			double s = ss * 0.9;
			initRandom((x + px) * ss, (z + pz) * ss);
			double x0 = (nextRandom(1024) / 1024.0 - 0.5) * s;
			double z0 = (nextRandom(1024) / 1024.0 - 0.5) * s;
			initRandom((x + px + 1) * ss, (z + pz) * ss);
			double x1 = (nextRandom(1024) / 1024.0 - 0.5) * s + ss;
			double z1 = (nextRandom(1024) / 1024.0 - 0.5) * s;
			initRandom((x + px) * ss, (z + pz + 1) * ss);
			double x2 = (nextRandom(1024) / 1024.0 - 0.5) * s;
			double z2 = (nextRandom(1024) / 1024.0 - 0.5) * s + ss;
			initRandom((x + px + 1) * ss, (z + pz + 1) * ss);
			double x3 = (nextRandom(1024) / 1024.0 - 0.5) * s + ss;
			double z3 = (nextRandom(1024) / 1024.0 - 0.5) * s + ss;
			int_t ur = p[x + 1 + (z + 0) * pw];
			int_t dr = p[x + 1 + (z + 1) * pw];
			for (int_t yy = 0; yy < ss; ++yy)
			{
				int_t pos = ((z << bits) + yy) * ww + (x << bits);
				for (int_t xx = 0; xx < ss; ++xx)
				{
					double d0 = (yy - z0) * (yy - z0) + (xx - x0) * (xx - x0);
					double d1 = (yy - z1) * (yy - z1) + (xx - x1) * (xx - x1);
					double d2 = (yy - z2) * (yy - z2) + (xx - x2) * (xx - x2);
					double d3 = (yy - z3) * (yy - z3) + (xx - x3) * (xx - x3);
					if (d0 < d1 && d0 < d2 && d0 < d3)
						tmp[pos++] = ul;
					else if (d1 < d0 && d1 < d2 && d1 < d3)
						tmp[pos++] = ur;
					else if (d2 < d0 && d2 < d1 && d2 < d3)
						tmp[pos++] = dl;
					else
						tmp[pos++] = dr;
				}
			}
			ul = ur;
			dl = dr;
		}
	}
	std::vector<int_t> result(static_cast<size_t>(w * h));
	copyRows(tmp, pw << bits, xo, zo, w, h, result, ss - 1);
	return result;
}
