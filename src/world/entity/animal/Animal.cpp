#include "world/entity/animal/Animal.h"

#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/GrassTile.h"
#include "util/Mth.h"

Animal::Animal(Level &level) : PathfinderMob(level)
{
}

int_t Animal::getAmbientSoundInterval()
{
	return 120;
}

float Animal::getWalkTargetValue(int_t x, int_t y, int_t z)
{
	return level.getTile(x, y - 1, z) == Tile::grass.id ? 10.0f : level.getBrightness(x, y, z) - 0.5f;
}

bool Animal::canSpawn()
{
	int_t x = Mth::floor(this->x);
	int_t y = Mth::floor(bb.y0);
	int_t z = Mth::floor(this->z);
	return level.getTile(x, y - 1, z) == Tile::grass.id && level.getFullBrightness(x, y, z) > 8 && PathfinderMob::canSpawn();
}
