#pragma once

#include "java/Type.h"

class Random
{
private:
	ulong_t seed;
	bool haveNextNextGaussian = false;
	double nextNextGaussian = 0.0;
public:
	Random();
	Random(long_t set_seed);

	void setSeed(long_t set_seed);

	int_t next(int_t bits);

	bool nextBoolean();

	int_t nextInt();
	int_t nextInt(int_t bound);

	long_t nextLong();

	float nextFloat();
	double nextDouble();
	double nextGaussian();
};
