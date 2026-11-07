#pragma once

#include "world/entity/Entity.h"

// 1.0 EnderEyeEntity: flies toward the nearest stronghold, then breaks or drops
class EyeOfEnderSignal : public Entity
{
	double targetX = 0.0;
	double targetY = 0.0;
	double targetZ = 0.0;
	int_t life = 0;
	bool surviveAfterDeath = false;

public:
	EyeOfEnderSignal(Level &level);
	EyeOfEnderSignal(Level &level, double x, double y, double z);
	jstring getEncodeId() const override { return u"EyeOfEnderSignal"; }

	void signalTo(double x, int_t y, double z);
	void lerpMotion(double xd, double yd, double zd) override;
	void tick() override;
	bool shouldRenderAtSqrDistance(double distance) override;
	float getBrightness(float a) override;
	float getShadowHeightOffs() override { return 0.0f; }

protected:
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
};
