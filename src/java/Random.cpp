#include "java/Random.h"
#include "java/Number.h"
#include "java/StrictMath.h"

#include <chrono>
#include <cmath>
#include <stdexcept>

static const ulong_t RANDOM_MUL = 0x5DEECE66DULL;
static const ulong_t RANDOM_ADD = 0xBULL;
static const ulong_t RANDOM_AND = (1ULL << 48) - 1;

Random::Random()
{
	auto now = std::chrono::high_resolution_clock::now();
	auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
	setSeed(static_cast<long_t>(nanos));
}

Random::Random(long_t set_seed)
{
	setSeed(set_seed);
}

void Random::setSeed(long_t set_seed)
{
	seed = (static_cast<ulong_t>(set_seed) ^ RANDOM_MUL) & RANDOM_AND;
	haveNextNextGaussian = false;
}

int_t Random::next(int_t bits)
{
	
	seed = (seed * RANDOM_MUL + RANDOM_ADD) & RANDOM_AND;
	return Java::intFromBits(static_cast<uint_t>(seed >> (48 - bits)));
}

bool Random::nextBoolean()
{
	return next(1) == 1;
}

int_t Random::nextInt()
{
	return next(32);
}

int_t Random::nextInt(int_t bound)
{
	if (bound <= 0)
		throw std::invalid_argument("bound must be positive");
	
	int_t r = next(31);
	int_t m = bound - 1;
	if ((bound & m) == 0)
	{
		r = static_cast<int_t>((bound * static_cast<long_t>(r)) >> 31);
	}
	else
	{
		int_t u = r;
		while (true)
		{
			r = u % bound;
			if (static_cast<uint_t>(u) - static_cast<uint_t>(r) + static_cast<uint_t>(m) < 0x80000000U)
				break;
			u = next(31);
		}
	}
	return r;
}

long_t Random::nextLong()
{
	
	int_t high = next(32);
	int_t low = next(32);
	ulong_t bits = (static_cast<ulong_t>(static_cast<long_t>(high)) << 32)
		+ static_cast<ulong_t>(static_cast<long_t>(low));
	return Java::longFromBits(bits);
}

float Random::nextFloat()
{
	return next(24) / static_cast<float>(1LL << 24);
}

double Random::nextDouble()
{
	int_t high = next(26);
	int_t low = next(27);
	return ((static_cast<ulong_t>(high) << 27) + static_cast<ulong_t>(low))
		/ static_cast<double>(1ULL << 53);
}


double Random::nextGaussian()
{
	if (haveNextNextGaussian)
	{
		haveNextNextGaussian = false;
		return nextNextGaussian;
	}

	double v1;
	double v2;
	double s;
	do
	{
		v1 = 2.0 * nextDouble() - 1.0;
		v2 = 2.0 * nextDouble() - 1.0;
		s = v1 * v1 + v2 * v2;
	} while (s >= 1.0 || s == 0.0);

	double multiplier = std::sqrt(-2.0 * StrictMath::log(s) / s);
	nextNextGaussian = v2 * multiplier;
	haveNextNextGaussian = true;
	return v1 * multiplier;
}