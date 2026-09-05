#include "world/level/tile/WorkbenchTile.h"

#include "client/player/LocalPlayer.h"
#include "world/level/Level.h"
#include "world/level/tile/WoodTile.h"

WorkbenchTile::WorkbenchTile(int_t id) : Tile(id, Material::wood)
{
	tex = 59;
}

int_t WorkbenchTile::getTexture(Facing face, int_t data)
{
	(void)data;
	if (face == Facing::UP)
		return tex - 16;
	if (face == Facing::DOWN)
		return Tile::wood.getTexture(Facing::DOWN);
	if (face == Facing::NORTH || face == Facing::SOUTH)
		return tex + 1;
	return tex;
}

bool WorkbenchTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	if (level.isOnline)
		return true;
	LocalPlayer *localPlayer = dynamic_cast<LocalPlayer *>(&player);
	if (localPlayer == nullptr)
		return false;
	localPlayer->startCrafting(x, y, z);
	return true;
}
