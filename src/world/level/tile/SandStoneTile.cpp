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

int_t SandStoneTile::getTexture(Facing face, int_t data)
{
	if (face == Facing::UP || (face == Facing::DOWN && (data == 1 || data == 2)))
		return 176;
	if (face == Facing::DOWN)
		return 208;
	if (data == 1)
		return 229;
	return data == 2 ? 230 : 192;
}
