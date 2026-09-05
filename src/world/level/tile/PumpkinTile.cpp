#include "world/level/tile/PumpkinTile.h"

#include "world/level/material/Material.h"

PumpkinTile::PumpkinTile(int_t id, int_t tex, bool lit) : Tile(id, tex, Material::pumpkin()), lit(lit)
{
	setTicking(true);
}

int_t PumpkinTile::getTexture(Facing face, int_t data)
{
	if (face == Facing::UP || face == Facing::DOWN)
		return tex;

	int_t frontTexture = tex + 17;
	if (lit)
		frontTexture++;

	if ((data == 2 && face == Facing::NORTH) ||
		(data == 3 && face == Facing::EAST) ||
		(data == 0 && face == Facing::SOUTH) ||
		(data == 1 && face == Facing::WEST))
	{
		return frontTexture;
	}

	return tex + 16;
}