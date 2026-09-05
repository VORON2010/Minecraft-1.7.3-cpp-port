#include "world/level/tile/SignTile.h"

#include "world/level/Level.h"
#include "world/level/tile/entity/SignTileEntity.h"
#include "world/item/Item.h"
#include "world/item/Items.h"
#include "util/Memory.h"

SignTile::SignTile(int_t id, bool freestanding) : Tile(id, 4, Material::wood), freestanding(freestanding)
{
	Tile::isEntityTile[id] = true;
	float r = 0.25f;
	setShape(0.5f - r, 0.0f, 0.5f - r, 0.5f + r, 1.0f, 0.5f + r);
	updateCachedProperties();
}

bool SignTile::isSolidRender()
{
	return false;
}

bool SignTile::isCubeShaped()
{
	return false;
}

Tile::Shape SignTile::getRenderShape()
{
	return SHAPE_INVISIBLE;
}

int_t SignTile::getResource(int_t data, Random &random)
{
	return Items::sign->getShiftedIndex();
}

void SignTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	Tile::onPlace(level, x, y, z);
	level.setTileEntity(x, y, z, Util::make_shared<SignTileEntity>());
}

void SignTile::onRemove(Level &level, int_t x, int_t y, int_t z)
{
	level.removeTileEntity(x, y, z);
	Tile::onRemove(level, x, y, z);
}

void SignTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t id)
{
	bool drop = false;
	if (freestanding)
	{
		if (!level.getMaterial(x, y - 1, z).isSolid())
			drop = true;
	}
	else
	{
		int_t data = level.getData(x, y, z);
		drop = true;
		if (data == 2 && level.getMaterial(x, y, z + 1).isSolid())
			drop = false;
		if (data == 3 && level.getMaterial(x, y, z - 1).isSolid())
			drop = false;
		if (data == 4 && level.getMaterial(x + 1, y, z).isSolid())
			drop = false;
		if (data == 5 && level.getMaterial(x - 1, y, z).isSolid())
			drop = false;
	}
	if (drop)
	{
		spawnResources(level, x, y, z, level.getData(x, y, z));
		level.setTile(x, y, z, 0);
	}
	Tile::neighborChanged(level, x, y, z, id);
}

AABB *SignTile::getTileAABB(Level &level, int_t x, int_t y, int_t z)
{
	updateShape(level, x, y, z);
	return Tile::getTileAABB(level, x, y, z);
}

void SignTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	if (freestanding)
	{
		float r = 0.25f;
		setShape(0.5f - r, 0.0f, 0.5f - r, 0.5f + r, 1.0f, 0.5f + r);
		return;
	}

	int_t data = level.getData(x, y, z);
	float top = 25.0f / 32.0f;
	float bot = 9.0f / 32.0f;
	float thick = 2.0f / 16.0f;
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	if (data == 2)
		setShape(0.0f, bot, 1.0f - thick, 1.0f, top, 1.0f);
	else if (data == 3)
		setShape(0.0f, bot, 0.0f, 1.0f, top, thick);
	else if (data == 4)
		setShape(1.0f - thick, bot, 0.0f, 1.0f, top, 1.0f);
	else if (data == 5)
		setShape(0.0f, bot, 0.0f, thick, top, 1.0f);
}

AABB *SignTile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	(void)level;
	(void)x;
	(void)y;
	(void)z;
	return nullptr;
}