#include "world/level/tile/EndPortalTile.h"

#include "util/Memory.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/dimension/Dimension.h"
#include "world/level/tile/entity/EndPortalTileEntity.h"

bool EndPortalTile::forcePlace = false;

EndPortalTile::EndPortalTile(int_t id, const Material &material) : Tile(id, 0, material)
{
	Tile::isEntityTile[id] = true;
	updateCachedProperties();
}

void EndPortalTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	(void)level;
	(void)x;
	(void)y;
	(void)z;
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.0625f, 1.0f);
}

bool EndPortalTile::shouldRenderFace(LevelSource &level, int_t x, int_t y, int_t z, Facing face)
{
	return face != Facing::DOWN ? false : Tile::shouldRenderFace(level, x, y, z, face);
}

void EndPortalTile::addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList)
{
	(void)level;
	(void)x;
	(void)y;
	(void)z;
	(void)bb;
	(void)aabbList;
}

bool EndPortalTile::isSolidRender()
{
	return false;
}

bool EndPortalTile::isCubeShaped()
{
	return false;
}

int_t EndPortalTile::getResourceCount(Random &random)
{
	(void)random;
	return 0;
}

void EndPortalTile::entityInside(Level &level, int_t x, int_t y, int_t z, Entity &entity)
{
	(void)x;
	(void)y;
	(void)z;
	if (entity.riding == nullptr && entity.rider == nullptr && !level.isOnline)
	{
		if (Player *player = dynamic_cast<Player *>(&entity))
			player->changeDimension(1);
	}
}

void EndPortalTile::animateTick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	double px = x + random.nextFloat();
	double py = y + 0.8f;
	double pz = z + random.nextFloat();
	level.addParticle(u"smoke", px, py, pz, 0.0, 0.0, 0.0);
}

void EndPortalTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	if (!forcePlace && level.dimension->id != 0)
	{
		level.setTile(x, y, z, 0);
		return;
	}
	level.setTileEntity(x, y, z, Util::make_shared<EndPortalTileEntity>());
}

void EndPortalTile::onRemove(Level &level, int_t x, int_t y, int_t z)
{
	level.removeTileEntity(x, y, z);
}
