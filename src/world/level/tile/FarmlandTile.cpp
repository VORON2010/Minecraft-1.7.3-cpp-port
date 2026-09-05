#include "world/level/tile/FarmlandTile.h"

#include "world/level/Level.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/level/material/Material.h"

namespace
{
	constexpr int_t DRY_TEXTURE = 87;
	constexpr int_t WET_TEXTURE = 86;
	constexpr int_t DIRT_TEXTURE = 2;
	constexpr int_t DIRT_TILE_ID = 3;
	constexpr int_t CROPS_TILE_ID = 59;
}

FarmlandTile::FarmlandTile(int_t id) : Tile(id, DRY_TEXTURE, Material::dirt)
{
	setTicking(true);
	updateDefaultShape();
	updateCachedProperties();
	setLightBlock(255);
}

bool FarmlandTile::isCubeShaped()
{
	return false;
}

int_t FarmlandTile::getTexture(Facing face, int_t data)
{
	return face == Facing::UP ? (data > 0 ? WET_TEXTURE : DRY_TEXTURE) : DIRT_TEXTURE;
}

AABB *FarmlandTile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	(void)level;
	return AABB::newTemp(x, y, z, x + 1, y + 1, z + 1);
}

bool FarmlandTile::isSolidRender()
{
	return false;
}

void FarmlandTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	if (random.nextInt(5) != 0)
		return;
	if (isWaterNearby(level, x, y, z))
	{
		if (level.getData(x, y, z) != 7)
			level.setData(x, y, z, 7);
		return;
	}

	int_t moisture = level.getData(x, y, z);
	if (moisture > 0)
	{
		level.setData(x, y, z, moisture - 1);
		return;
	}
	if (!hasCropAbove(level, x, y, z))
		level.setTile(x, y, z, DIRT_TILE_ID);
}

void FarmlandTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	(void)tile;
	if (level.getMaterial(x, y + 1, z).isSolid())
		level.setTile(x, y, z, DIRT_TILE_ID);
}

int_t FarmlandTile::getResource(int_t data, Random &random)
{
	(void)data;
	(void)random;
	return DIRT_TILE_ID;
}

void FarmlandTile::stepOn(Level &level, int_t x, int_t y, int_t z, Entity &entity)
{
	(void)entity;
	if (!level.isOnline && level.random.nextInt(4) == 0)
		level.setTile(x, y, z, DIRT_TILE_ID);
}

void FarmlandTile::updateDefaultShape()
{
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 15.0f / 16.0f, 1.0f);
}

bool FarmlandTile::isWaterNearby(Level &level, int_t x, int_t y, int_t z) const
{
	const Material &water = Material::water;
	for (int_t xx = x - 4; xx <= x + 4; ++xx)
	{
		for (int_t yy = y; yy <= y + 1; ++yy)
		{
			for (int_t zz = z - 4; zz <= z + 4; ++zz)
			{
				if (&level.getMaterial(xx, yy, zz) == &water)
					return true;
			}
		}
	}
	return false;
}

bool FarmlandTile::hasCropAbove(Level &level, int_t x, int_t y, int_t z) const
{
	return level.getTile(x, y + 1, z) == CROPS_TILE_ID;
}
