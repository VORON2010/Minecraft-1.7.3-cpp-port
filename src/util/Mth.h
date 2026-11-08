#pragma once

#include <cmath>

#include "java/Type.h"
#include "java/Random.h"

namespace Mth
{

static constexpr float PI = 3.1415927f;
static constexpr float RAD_TO_GRAD = 0.017453292f;
static constexpr float DEGRAD = 0.017453292f;
static constexpr float RADDEG = 57.295776f;

float sin(float angle);
float cos(float angle);

float sqrt(float value);
float sqrt(double value);

int_t floor(float value);
int_t floor(double value);

float abs(float value);

int_t ceil(float value);

double asbMax(double a, double b);

int_t intFloorDiv(int_t a, int_t b);

// 1.5.2 MathHelper.wrapDegrees: into [-180, 180)
inline float wrapDegrees(float value)
{
	value = static_cast<float>(std::fmod(value, 360.0f));
	if (value >= 180.0f)
		value -= 360.0f;
	if (value < -180.0f)
		value += 360.0f;
	return value;
}

inline double wrapDegrees(double value)
{
	value = std::fmod(value, 360.0);
	if (value >= 180.0)
		value -= 360.0;
	if (value < -180.0)
		value += 360.0;
	return value;
}

inline int_t clamp(int_t value, int_t min, int_t max) { return value < min ? min : (value > max ? max : value); }
inline int_t nextInt(Random &random, int_t min, int_t max) { return min >= max ? min : random.nextInt(max - min + 1) + min; }

}
