#include "util/Mth.h"

#include "java/Number.h"

#include <cmath>
#include <cstring>
#include <stdexcept>

namespace Mth
{

struct SinTable
{
	float table[65536];

	SinTable()
	{
		for (int_t i = 0; i < 65536; i++)
			table[i] = static_cast<float>(std::sin(i * 3.141592653589793 * 2.0 / 65536.0));
	}
} static const SIN_TABLE;


float sin(float angle)
{
	return SIN_TABLE.table[Java::numberToInt(angle * 10430.378f) & 65535];
}
float cos(float angle)
{
	return SIN_TABLE.table[Java::numberToInt(angle * 10430.378f + 16384.0f) & 65535];
}

float sqrt(float value)
{
	return static_cast<float>(std::sqrt(static_cast<double>(value)));
}
float sqrt(double value)
{
	return static_cast<float>(std::sqrt(value));
}

int_t floor(float value)
{
	int_t i = Java::numberToInt(value);
	return (value < i) ? Java::intFromBits(static_cast<uint_t>(i) - 1U) : i;
}
int_t floor(double value)
{
	int_t i = Java::numberToInt(value);
	return (value < i) ? Java::intFromBits(static_cast<uint_t>(i) - 1U) : i;
}

float abs(float value)
{
	
	uint_t bits;
	std::memcpy(&bits, &value, sizeof(bits));
	if (!(value >= 0.0f))
		bits ^= 0x80000000U;
	std::memcpy(&value, &bits, sizeof(value));
	return value;
}

int_t ceil(float value)
{
	return Java::numberToInt(std::ceil(static_cast<double>(value)));
}


double asbMax(double a, double b)
{
	if (a < 0)
		a = -a;
	if (b < 0)
		b = -b;
	return (a > b) ? a : b;
}

int_t intFloorDiv(int_t a, int_t b)
{
	if (b == 0)
		throw std::domain_error("/ by zero");
	if (a < 0)
	{
		
		const int_t numerator = -(a + 1);
		return -(numerator / b) - 1;
	}
	return a / b;
}

}
