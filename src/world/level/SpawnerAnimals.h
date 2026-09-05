#pragma once

#include "java/Type.h"

class Level;

class SpawnerAnimals
{
public:
	static int_t performSpawning(Level &level, bool spawnHostileMobs, bool spawnPeacefulMobs);
};
