#include "world/level/tile/PistonMovingTile.h"

#include "world/level/Level.h"
#include "world/level/tile/PistonTextures.h"
#include "world/level/tile/entity/PistonTileEntity.h"
#include "world/level/tile/Tile.h"
#include "world/entity/Entity.h"
#include "world/entity/player/Player.h"
#include "world/phys/AABB.h"

PistonMovingTile::PistonMovingTile(int_t id) : Tile(id, Material::piston)
{
	setDestroyTime(-1.0f);
	updateCachedProperties();
	Tile::isEntityTile[id] = true;
}

Tile::Shape PistonMovingTile::getRenderShape()
{
	return SHAPE_INVISIBLE;
}

bool PistonMovingTile::isSolidRender()
{
	return false;
}

bool PistonMovingTile::mayPlace(Level &level, int_t x, int_t y, int_t z)
{
	(void)level; (void)x; (void)y; (void)z;
	return false;
}

void PistonMovingTile::setPlacedOnFace(Level &level, int_t x, int_t y, int_t z, Facing face)
{
	(void)level; (void)x; (void)y; (void)z; (void)face;
}

int_t PistonMovingTile::getResource(int_t data, Random &random)
{
	(void)random;
	return 0;
}

void PistonMovingTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	(void)level; (void)x; (void)y; (void)z; (void)tile;
}

void PistonMovingTile::onRemove(Level &level, int_t x, int_t y, int_t z)
{
	PistonTileEntity *te = getPistonTileEntity(level, x, y, z);
	if (te != nullptr)
		te->finishMovement();
	else
		Tile::onRemove(level, x, y, z);
}

void PistonMovingTile::spawnResources(Level &level, int_t x, int_t y, int_t z, int_t data, float chance)
{
	(void)data; (void)chance;
	PistonTileEntity *te = getPistonTileEntity(level, x, y, z);
	if (te != nullptr)
	{
		Tile *stored = Tile::tiles[te->getStoredBlockID()];
		if (stored != nullptr)
			stored->spawnResources(level, x, y, z, te->getBlockMetadata());
	}
}

bool PistonMovingTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	(void)player;
	if (!level.isOnline && level.getTileEntity(x, y, z) == nullptr)
	{
		level.setTile(x, y, z, 0);
		return true;
	}
	return false;
}

AABB *PistonMovingTile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	PistonTileEntity *te = getPistonTileEntity(level, x, y, z);
	if (te == nullptr)
		return nullptr;
	float progress = te->getProgress(0.0f);
	if (te->isExtending())
		progress = 1.0f - progress;
	return getPushedAABB(level, x, y, z, te->getStoredBlockID(), progress, te->getDirection());
}

void PistonMovingTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	PistonTileEntity *te = getPistonTileEntity(level, x, y, z);
	if (te == nullptr)
		return;
	Tile *stored = Tile::tiles[te->getStoredBlockID()];
	if (stored == nullptr || stored == this)
		return;
	stored->updateShape(level, x, y, z);
	float progress = te->getProgress(0.0f);
	if (te->isExtending())
		progress = 1.0f - progress;
	int_t dir = te->getDirection();
	xx0 = stored->xx0 - PistonTextures::offsetX[dir] * progress;
	yy0 = stored->yy0 - PistonTextures::offsetY[dir] * progress;
	zz0 = stored->zz0 - PistonTextures::offsetZ[dir] * progress;
	xx1 = stored->xx1 - PistonTextures::offsetX[dir] * progress;
	yy1 = stored->yy1 - PistonTextures::offsetY[dir] * progress;
	zz1 = stored->zz1 - PistonTextures::offsetZ[dir] * progress;
}

std::shared_ptr<TileEntity> PistonMovingTile::createTileEntity(int_t blockId, int_t blockData, int_t facing, bool extending, bool renderHead)
{
	return std::make_shared<PistonTileEntity>(blockId, blockData, facing, extending, renderHead);
}

PistonTileEntity *PistonMovingTile::getPistonTileEntity(LevelSource &level, int_t x, int_t y, int_t z)
{
	auto te = level.getTileEntity(x, y, z);
	if (te != nullptr)
		return dynamic_cast<PistonTileEntity *>(te.get());
	return nullptr;
}

AABB *PistonMovingTile::getPushedAABB(Level &level, int_t x, int_t y, int_t z, int_t blockId, float progress, int_t dir)
{
	if (blockId == 0 || blockId == this->id)
		return nullptr;
	Tile *stored = Tile::tiles[blockId];
	if (stored == nullptr)
		return nullptr;
	AABB *aabb = stored->getAABB(level, x, y, z);
	if (aabb == nullptr)
		return nullptr;
	double dx = PistonTextures::offsetX[dir] * progress;
	double dy = PistonTextures::offsetY[dir] * progress;
	double dz = PistonTextures::offsetZ[dir] * progress;
	return AABB::newTemp(aabb->x0 - dx, aabb->y0 - dy, aabb->z0 - dz, aabb->x1 - dx, aabb->y1 - dy, aabb->z1 - dz);
}