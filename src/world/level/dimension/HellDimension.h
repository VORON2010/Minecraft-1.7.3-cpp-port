#pragma once

#include "world/level/dimension/Dimension.h"


class HellDimension : public Dimension
{
public:
	HellDimension(Level &level);

protected:
	void updateLightRamp() override;

public:
	ChunkSource *createRandomLevelSource() override;
	ChunkStorage *createStorage(std::shared_ptr<File> dir) override;

	bool isValidSpawn(int_t x, int_t z) override;
	float getTimeOfDay(long_t time, float add) override;
	Vec3 *getFogColor(float time, float unknown) override;
	bool mayRespawn() override;
};
