#pragma once

#include <cmath>
#include <cstring>
#include <limits>

#include "java/Type.h"

namespace Java
{

inline int_t intFromBits(uint_t bits)
{
	int_t result;
	std::memcpy(&result, &bits, sizeof(result));
	return result;
}

inline long_t longFromBits(ulong_t bits)
{
	long_t result;
	std::memcpy(&result, &bits, sizeof(result));
	return result;
}

inline int_t numberToInt(double value)
{
	if (std::isnan(value))
		return 0;
	if (value <= static_cast<double>(std::numeric_limits<int_t>::min()))
		return std::numeric_limits<int_t>::min();
	if (value >= static_cast<double>(std::numeric_limits<int_t>::max()))
		return std::numeric_limits<int_t>::max();
	return static_cast<int_t>(value);
}

}
