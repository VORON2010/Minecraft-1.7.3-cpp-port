#include "world/level/tile/FenceGateTile.h"

#include "util/Mth.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"

FenceGateTile::FenceGateTile(int_t id, int_t tex) : Tile(id, tex, Material::wood)
{
	updateCachedProperties();
}

bool FenceGateTile::mayPlace(Level &level, int_t x, int_t y, int_t z)
{
	if (!level.getMaterial(x, y - 1, z).isSolid())
		return false;
	return Tile::mayPlace(level, x, y, z);
}

AABB *FenceGateTile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	if (isOpen(level.getData(x, y, z)))
		return nullptr;
	return AABB::newTemp(x, y, z, x + 1, static_cast<float>(y) + 1.5f, z + 1);
}

bool FenceGateTile::isCubeShaped()
{
	return false;
}

bool FenceGateTile::isSolidRender()
{
	return false;
}

void FenceGateTile::setPlacedBy(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	int_t facing = (Mth::floor(static_cast<double>(player.yRot * 4.0f / 360.0f) + 0.5) & 3) % 4;
	level.setData(x, y, z, facing);
}

bool FenceGateTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	int_t data = level.getData(x, y, z);
	if (isOpen(data))
	{
		level.setData(x, y, z, data & ~4);
	}
	else
	{
		int_t facing = (Mth::floor(static_cast<double>(player.yRot * 4.0f / 360.0f) + 0.5) & 3) % 4;
		if (getFacing(data) == (facing + 2) % 4)
			data = facing;
		level.setData(x, y, z, data | 4);
	}
	level.levelEvent(&player, 1003, x, y, z, 0);
	return true;
}
