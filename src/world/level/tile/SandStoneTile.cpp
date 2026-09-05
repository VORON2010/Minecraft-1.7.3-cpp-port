#include "world/level/tile/SandStoneTile.h"

SandStoneTile::SandStoneTile(int_t id, int_t texture) : Tile(id, texture, Material::stone)
{
}

int_t SandStoneTile::getTexture(Facing face)
{
	if (face == Facing::UP)
		return tex - 16;
	if (face == Facing::DOWN)
		return tex + 16;
	return tex;
}
