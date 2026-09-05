#include "world/level/tile/MobSpawnerTile.h"

#include "world/level/Level.h"
#include "world/level/tile/entity/MobSpawnerTileEntity.h"
#include "util/Memory.h"

MobSpawnerTile::MobSpawnerTile(int_t id, int_t tex) : Tile(id, tex, Material::stone)
{
	Tile::isEntityTile[id] = true;
	updateCachedProperties();
}

bool MobSpawnerTile::isSolidRender()
{
	return false;
}

int_t MobSpawnerTile::getResource(int_t data, Random &random)
{
	(void)data;
	(void)random;
	return 0;
}

int_t MobSpawnerTile::getResourceCount(Random &random)
{
	(void)random;
	return 0;
}

void MobSpawnerTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	level.setTileEntity(x, y, z, Util::make_shared<MobSpawnerTileEntity>());
}

void MobSpawnerTile::onRemove(Level &level, int_t x, int_t y, int_t z)
{
	level.removeTileEntity(x, y, z);
}
