#include "world/level/tile/WoodTile.h"

WoodTile::WoodTile(int_t id, int_t tex) : Tile(id, tex, Material::wood)
{
}

int_t WoodTile::getTexture(Facing face, int_t data)
{
	(void)face;
	switch (data)
	{
	case 1: return 198;
	case 2: return 214;
	case 3: return 199;
	default: return 4;
	}
}
