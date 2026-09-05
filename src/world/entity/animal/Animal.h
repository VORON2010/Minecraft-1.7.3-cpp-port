#pragma once

#include "world/entity/PathfinderMob.h"

class Animal : public PathfinderMob
{
public:
	Animal(Level &level);
	int_t getAmbientSoundInterval() override;
	bool canSpawn() override;

protected:
	float getWalkTargetValue(int_t x, int_t y, int_t z) override;
};
