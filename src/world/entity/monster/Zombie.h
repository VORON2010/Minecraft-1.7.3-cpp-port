#pragma once

#include "world/entity/monster/Monster.h"

class Zombie : public Monster
{
public:
	Zombie(Level &level);
	jstring getEncodeId() const override { return u"Zombie"; }
	void aiStep() override;

protected:
	jstring getAmbientSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDeathLoot() override;
};
