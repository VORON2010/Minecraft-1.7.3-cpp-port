#pragma once

#include "world/entity/Entity.h"

class EntityLightningBolt : public Entity
{
private:
	int_t life = 2;
	int_t flashes = 0;

public:
	long_t randomSeed = 0;

	EntityLightningBolt(Level &level);
	EntityLightningBolt(Level &level, double x, double y, double z);

	jstring getEncodeId() const override { return u"LightningBolt"; }
	void tick() override;
	bool shouldRenderAtSqrDistance(double distance) override;

protected:
	void readAdditionalSaveData(CompoundTag &tag) override;
	void addAdditionalSaveData(CompoundTag &tag) override;
};
