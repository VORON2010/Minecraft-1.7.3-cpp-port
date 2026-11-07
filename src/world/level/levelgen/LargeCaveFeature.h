#pragma once

#include "world/level/levelgen/LargeFeature.h"

// 1.0 cave carver
class LargeCaveFeature : public LargeFeature
{
protected:
	void addRoom(long_t seed, int_t chunkX, int_t chunkZ, Blocks &blocks, double x, double y, double z);
	void addTunnel(long_t seed, int_t chunkX, int_t chunkZ, Blocks &blocks, double x, double y, double z, float baseWidth, float yaw, float pitch, int_t tunnel, int_t tunnelCount, double widthHeightRatio);
	void addFeature(Level &level, int_t startChunkX, int_t startChunkZ, int_t chunkX, int_t chunkZ, Blocks &blocks) override;
};
