#pragma once

#include <array>

#include "java/Type.h"
#include "java/Random.h"

class Level;
class ChunkSource;

// Base for carvers that run over the chunks around the one being generated (caves, ravines).
// Seeding follows 1.0 (MapGenBase).
class LargeFeature
{
public:
	using Blocks = std::array<ubyte_t, 16 * 16 * 128>;

protected:
	int_t radius = 8;
	Random random = Random();
	Level *level = nullptr;

public:
	virtual ~LargeFeature() {}

	void apply(ChunkSource &chunkSource, Level &level, int_t x, int_t z, Blocks &blocks);

protected:
	virtual void addFeature(Level &level, int_t xx, int_t zz, int_t x, int_t z, Blocks &blocks);
};
