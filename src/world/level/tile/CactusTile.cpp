#include "world/level/tile/CactusTile.h"

#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/tile/SandTile.h"

CactusTile::CactusTile(int_t id, int_t tex) : Tile(id, tex, Material::cactus())
{
	setTicking(true);
	updateCachedProperties();
}

bool CactusTile::isCubeShaped()
{
	return false;
}

Tile::Shape CactusTile::getRenderShape()
{
	return SHAPE_CACTUS;
}

AABB *CactusTile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	float inset = 1.0f / 16.0f;
	return AABB::newTemp(x + inset, y, z + inset, x + 1.0f - inset, y + 1.0f - inset, z + 1.0f - inset);
}

AABB *CactusTile::getTileAABB(Level &level, int_t x, int_t y, int_t z)
{
	float inset = 1.0f / 16.0f;
	return AABB::newTemp(x + inset, y, z + inset, x + 1.0f - inset, y + 1.0f, z + 1.0f - inset);
}

bool CactusTile::isSolidRender()
{
	return false;
}

int_t CactusTile::getTexture(Facing face)
{
	if (face == Facing::UP)
		return tex - 1;
	if (face == Facing::DOWN)
		return tex + 1;
	return tex;
}

void CactusTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
	{
		if (!level.isEmptyTile(x, y + 1, z))
			return;
	
		int_t height = 1;
		while (level.getTile(x, y - height, z) == id)
			++height;
	
		if (height >= 3)
			return;
	
		int_t data = level.getData(x, y, z);
		if (data == 15)
		{
			level.setTile(x, y + 1, z, id);
			level.setData(x, y, z, 0);
		}
		else
		{
			level.setData(x, y, z, data + 1);
		}
	}
	
void CactusTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	if (!canStay(level, x, y, z))
		level.setTile(x, y, z, 0);
}

void CactusTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	if (!canStay(level, x, y, z))
		level.setTile(x, y, z, 0);
}
	
void CactusTile::entityInside(Level &level, int_t x, int_t y, int_t z, Entity &entity)
{
	entity.hurt(nullptr, 1);
}


bool CactusTile::canStay(Level &level, int_t x, int_t y, int_t z)
{
	if (level.getMaterial(x - 1, y, z).isSolid())
		return false;
	if (level.getMaterial(x + 1, y, z).isSolid())
		return false;
	if (level.getMaterial(x, y, z - 1).isSolid())
		return false;
	if (level.getMaterial(x, y, z + 1).isSolid())
		return false;

	int_t belowTile = level.getTile(x, y - 1, z);
	return belowTile == id || belowTile == Tile::sand.id;
}
