#include "world/level/tile/StoneBrickTile.h"

StoneBrickTile::StoneBrickTile(int_t id) : Tile(id, TEX_NORMAL, Material::stone)
{
	updateCachedProperties();
}

int_t StoneBrickTile::getTexture(Facing face, int_t data)
{
	(void)face;
	switch (data)
	{
	case 1: return TEX_MOSSY;
	case 2: return TEX_CRACKED;
	default: return TEX_NORMAL;
	}
}
