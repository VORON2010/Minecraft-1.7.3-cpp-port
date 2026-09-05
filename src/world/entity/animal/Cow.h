#pragma once

#include "world/entity/animal/Animal.h"

class Cow : public Animal
{
public:
	Cow(Level &level);
	jstring getEncodeId() const override { return u"Cow"; }
	bool interact(Player &player) override;

protected:
	jstring getAmbientSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	float getSoundVolume() override;
	int_t getDeathLoot() override;
};
