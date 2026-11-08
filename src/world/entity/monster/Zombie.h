#pragma once

#include "world/entity/monster/Monster.h"

class Zombie : public Monster
{
public:
	void playStepSound(int_t x, int_t y, int_t z, int_t tile) override;
	bool isUndead() override { return true; }
	Zombie(Level &level);
	jstring getEncodeId() const override { return u"Zombie"; }
	void aiStep() override;

protected:
	jstring getAmbientSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDeathLoot() override;
};
