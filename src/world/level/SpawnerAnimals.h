#pragma once

#include "java/Type.h"

class Level;
class Biome;
class Random;

class SpawnerAnimals
{
public:
	static int_t performSpawning(Level &level, bool spawnHostileMobs, bool spawnPeacefulMobs);
	static void postProcessSpawnMobs(Level &level, Biome &biome, int_t xo, int_t zo, int_t width, int_t depth, Random &random);
};
