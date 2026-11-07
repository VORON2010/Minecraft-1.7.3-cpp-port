#pragma once

#include <array>

#include "world/level/levelgen/LargeFeature.h"

// b1.8 ravines
class RavineFeature : public LargeFeature
{
private:
	std::array<float, 1024> buffer = {};

protected:
	void addRavine(long_t seed, int_t chunkX, int_t chunkZ, Blocks &blocks, double x, double y, double z, float baseWidth, float yaw, float pitch, int_t tunnel, int_t tunnelCount, double widthHeightRatio);
	void addFeature(Level &level, int_t startChunkX, int_t startChunkZ, int_t chunkX, int_t chunkZ, Blocks &blocks) override;
};
