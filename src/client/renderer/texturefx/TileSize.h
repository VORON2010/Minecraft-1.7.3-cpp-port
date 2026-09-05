#pragma once

#include "java/Type.h"

namespace TileSize
{
	extern int_t size;
	extern int_t sizeMinus1;
	extern int_t sizeHalf;
	extern int_t numPixels;
	extern int_t numBytes;
	extern int_t numPixelsMinus1;
	extern int_t compassNeedleMin;
	extern int_t compassNeedleMax;
	extern int_t compassCrossMin;
	extern int_t compassCrossMax;
	extern int_t flameHeight;
	extern int_t flameHeightMinus1;
	extern int_t flameArraySize;

	extern float sizeFloat;
	extern float sizeMinus0_01;
	extern float size16;
	extern float reciprocal;
	extern float texNudge;
	extern float flameNudge;

	extern double sizeDouble;
	extern double sizeMinus1Double;
	extern double compassCenterMin;
	extern double compassCenterMax;

	void set(int_t value);
}
