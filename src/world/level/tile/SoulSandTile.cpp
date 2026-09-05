#include "world/level/tile/SoulSandTile.h"

#include "world/entity/Entity.h"

SoulSandTile::SoulSandTile(int_t id, int_t tex, const Material &material) : Tile(id, tex, material)
{
}

void SoulSandTile::entityInside(Level &level, int_t x, int_t y, int_t z, Entity &entity)
{
	entity.xd *= 0.4;
	entity.zd *= 0.4;
}

AABB *SoulSandTile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	return AABB::newTemp(x, y, z, x + 1, static_cast<float>(y + 1) - 0.125f, z + 1);
}