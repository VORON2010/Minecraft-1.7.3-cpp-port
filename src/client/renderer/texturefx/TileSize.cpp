#include "client/renderer/texturefx/TileSize.h"

namespace TileSize
{
	int_t size;
	int_t sizeMinus1;
	int_t sizeHalf;
	int_t numPixels;
	int_t numBytes;
	int_t numPixelsMinus1;
	int_t compassNeedleMin;
	int_t compassNeedleMax;
	int_t compassCrossMin;
	int_t compassCrossMax;
	int_t flameHeight;
	int_t flameHeightMinus1;
	int_t flameArraySize;

	float sizeFloat;
	float sizeMinus0_01;
	float size16;
	float reciprocal;
	float texNudge;
	float flameNudge;

	double sizeDouble;
	double sizeMinus1Double;
	double compassCenterMin;
	double compassCenterMax;

	void set(int_t value)
	{
		size = value;
		sizeMinus1 = value - 1;
		sizeHalf = value / 2;
		numPixels = value * value;
		numBytes = 4 * numPixels;
		numPixelsMinus1 = numPixels - 1;
		compassNeedleMin = value / -2;
		compassNeedleMax = value;
		compassCrossMin = value / -4;
		compassCrossMax = value / 4;
		flameHeight = value + 4;
		flameHeightMinus1 = flameHeight - 1;
		flameArraySize = value * flameHeight;

		sizeFloat = static_cast<float>(value);
		sizeMinus0_01 = sizeFloat - 0.01f;
		size16 = sizeFloat * 16.0f;
		reciprocal = 1.0f / sizeFloat;
		texNudge = 1.0f / (sizeFloat * sizeFloat * 2.0f);
		flameNudge = value < 64 ? 1.0f + 0.96f / sizeFloat : 1.0f + 1.28f / sizeFloat;

		sizeDouble = static_cast<double>(value);
		sizeMinus1Double = sizeDouble - 1.0;
		compassCenterMin = sizeDouble / 2.0 - 0.5;
		compassCenterMax = sizeDouble / 2.0 + 0.5;
	}

	namespace
	{
		struct Initializer
		{
			Initializer()
			{
				set(16);
			}
		} initializer;
	}
}
