#include "world/level/tile/FenceTile.h"

FenceTile::FenceTile(int_t id, int_t tex, const Material &material) : Tile(id, tex, material)
{
	updateCachedProperties();
}

bool FenceTile::isCubeShaped()
{
	return false;
}

bool FenceTile::isSolidRender()
{
	return false;
}

Tile::Shape FenceTile::getRenderShape()
{
	return SHAPE_FENCE;
}

AABB *FenceTile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	return AABB::newTemp(x, y, z, x + 1, static_cast<float>(y) + 1.5f, z + 1);
}