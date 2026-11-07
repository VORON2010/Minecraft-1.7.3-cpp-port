#pragma once

#include "world/entity/monster/Monster.h"

// 1.0 blaze: hovers, shoots three small fireballs in a burst
class Blaze : public Monster
{
	float allowedHeightOffset = 0.5f;
	int_t nextHeightOffsetChangeTick = 0;
	int_t attackCounter = 0;

public:
	Blaze(Level &level);
	jstring getEncodeId() const override { return u"Blaze"; }
	float getBrightness(float a) override;
	void aiStep() override;
	bool isOnFire() override;
	bool canSpawn() override;
	bool isCharged();
	void setCharged(bool charged);

protected:
	jstring getAmbientSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	void checkHurtTarget(Entity &entity, float distance) override;
	void causeFallDamage(float distance) override;
	int_t getDeathLoot() override;
	void dropDeathLoot() override;
};
