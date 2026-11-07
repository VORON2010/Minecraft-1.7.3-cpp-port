#include "world/level/levelgen/LargeFeature.h"
#include "java/Number.h"

#include "world/level/Level.h"
#include "world/level/chunk/ChunkSource.h"

void LargeFeature::apply(ChunkSource &chunkSource, Level &level, int_t x, int_t z, Blocks &blocks)
{
	(void)chunkSource;
	int_t radius = this->radius;
	this->level = &level;

	random.setSeed(level.seed);
	long_t cs0 = random.nextLong();
	long_t cs1 = random.nextLong();

	for (int_t xx = x - radius; xx <= x + radius; xx++)
	{
		for (int_t zz = z - radius; zz <= z + radius; zz++)
		{
			ulong_t a = static_cast<ulong_t>(static_cast<long_t>(xx)) * static_cast<ulong_t>(cs0);
			ulong_t b = static_cast<ulong_t>(static_cast<long_t>(zz)) * static_cast<ulong_t>(cs1);
			random.setSeed(Java::longFromBits(a ^ b ^ static_cast<ulong_t>(level.seed)));
			addFeature(level, xx, zz, x, z, blocks);
		}
	}
}

void LargeFeature::addFeature(Level &level, int_t xx, int_t zz, int_t x, int_t z, Blocks &blocks)
{
	(void)level; (void)xx; (void)zz; (void)x; (void)z; (void)blocks;
}
