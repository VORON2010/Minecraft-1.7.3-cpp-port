#pragma once

#include "world/level/dimension/Dimension.h"

// 1.0 TheEndDimension: no sky light, no day cycle, the player arrives at 100,50,0
class TheEndDimension : public Dimension
{
public:
	TheEndDimension(Level &level);

protected:
	void updateLightRamp() override;

public:

	ChunkSource *createRandomLevelSource() override;
	ChunkStorage *createStorage(std::shared_ptr<File> dir) override;

	float getTimeOfDay(long_t time, float add) override;
	float *getSunriseColor(float time, float unknown) override;
	Vec3 *getFogColor(float time, float unknown) override;
	bool hasGround() override;
	bool mayRespawn() override;
	float getCloudHeight() override;
	bool isValidSpawn(int_t x, int_t z) override;
	bool getForcedSpawnPoint(int_t &x, int_t &y, int_t &z) override;
};
