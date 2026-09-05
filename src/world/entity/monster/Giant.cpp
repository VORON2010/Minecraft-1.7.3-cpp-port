#include "world/entity/monster/Giant.h"

#include "world/level/Level.h"

Giant::Giant(Level &level) : Zombie(level)
{
	attackDamage = 50;
	health *= 10;
	setSize(bbWidth * 6.0f, bbHeight * 6.0f);
}

float Giant::getWalkTargetValue(int_t x, int_t y, int_t z)
{
	return level.getBrightness(x, y, z) - 0.5f;
}
