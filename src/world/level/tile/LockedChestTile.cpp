#include "world/level/tile/LockedChestTile.h"

#include "world/level/Level.h"
#include "world/level/material/Material.h"

LockedChestTile::LockedChestTile(int_t id) : Tile(id, 26, Material::wood)
{
	setTicking(true);
}

int_t LockedChestTile::getTexture(LevelSource &level, int_t x, int_t y, int_t z, Facing face)
{
	if (face == Facing::UP || face == Facing::DOWN)
		return tex - 1;

	bool north = level.isSolidTile(x, y, z - 1);
	bool south = level.isSolidTile(x, y, z + 1);
	bool west = level.isSolidTile(x - 1, y, z);
	bool east = level.isSolidTile(x + 1, y, z);
	Facing front = Facing::SOUTH;
	if (north && !south)
		front = Facing::SOUTH;
	if (south && !north)
		front = Facing::NORTH;
	if (west && !east)
		front = Facing::EAST;
	if (east && !west)
		front = Facing::WEST;
	return face == front ? tex + 1 : tex;
}

int_t LockedChestTile::getTexture(Facing face, int_t data)
{
	(void)data;
	if (face == Facing::UP || face == Facing::DOWN)
		return tex - 1;
	return face == Facing::SOUTH ? tex + 1 : tex;
}

void LockedChestTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	(void)random;
	level.setTile(x, y, z, 0);
}
