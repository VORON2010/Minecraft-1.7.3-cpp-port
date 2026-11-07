#include "world/level/levelgen/synth/PerlinNoise.h"

PerlinNoise::PerlinNoise(int_t levels) : PerlinNoise(Random(), levels)
{

}

PerlinNoise::PerlinNoise(Random &random, int_t levels)
{
	this->levels = levels;

	for (int_t i = 0; i < levels; i++)
		noiseLevels.emplace_back(random);
}

PerlinNoise::PerlinNoise(Random &&random, int_t levels) : PerlinNoise(random, levels)
{

}

double PerlinNoise::getValue(double x, double y)
{
	double result = 0.0;
	double scale = 1.0;

	for (auto &i : noiseLevels)
	{
		result += i.getValue(x * scale, y * scale) / scale;
		scale /= 2.0;
	}

	return result;
}

double PerlinNoise::getValue(double x, double y, double z)
{
	double result = 0.0;
	double scale = 1.0;

	for (auto &i : noiseLevels)
	{
		result += i.getValue(x * scale, y * scale, z * scale) / scale;
		scale /= 2.0;
	}

	return result;
}

void PerlinNoise::getRegion(double *out, int_t x, int_t y, int_t z, int_t xSize, int_t ySize, int_t zSize, double xScale, double yScale, double zScale)
{
	std::fill(out, out + xSize * ySize * zSize, 0.0);

	double scale = 1.0;
	for (auto &i : noiseLevels)
	{
		double xx = static_cast<double>(x) * scale * xScale;
		double yy = static_cast<double>(y) * scale * yScale;
		double zz = static_cast<double>(z) * scale * zScale;
		long_t xl = static_cast<long_t>(xx);
		if (xx < static_cast<double>(xl))
			xl--;
		long_t zl = static_cast<long_t>(zz);
		if (zz < static_cast<double>(zl))
			zl--;
		xx -= static_cast<double>(xl);
		zz -= static_cast<double>(zl);
		xl %= 16777216LL;
		zl %= 16777216LL;
		xx += static_cast<double>(xl);
		zz += static_cast<double>(zl);
		i.add(out, xx, yy, zz, xSize, ySize, zSize, xScale * scale, yScale * scale, zScale * scale, scale);
		scale /= 2.0;
	}
}

void PerlinNoise::getRegion(double *out, int_t x, int_t z, int_t xSize, int_t zSize, double xScale, double zScale, double unknown)
{
	(void)unknown;
	getRegion(out, x, 10, z, xSize, 1, zSize, xScale, 1.0, zScale);
}
