#pragma once

#include "world/entity/animal/Animal.h"

class Chicken : public Animal
{
public:
	float flap = 0.0f;
	float flapSpeed = 0.0f;
	float oFlapSpeed = 0.0f;
	float oFlap = 0.0f;
	float flapping = 1.0f;
	int_t eggTime = 0;

	Chicken(Level &level);
	jstring getEncodeId() const override { return u"Chicken"; }
	void aiStep() override;

protected:
	void causeFallDamage(float distance) override;
	jstring getAmbientSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDeathLoot() override;
};
