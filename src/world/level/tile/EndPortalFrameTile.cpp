#include "world/level/tile/EndPortalFrameTile.h"

#include "util/Mth.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"

EndPortalFrameTile::EndPortalFrameTile(int_t id) : Tile(id, 159, Material::glass)
{
}

int_t EndPortalFrameTile::getTexture(Facing face, int_t data)
{
	(void)data;
	if (face == Facing::UP)
		return tex - 1;
	return face == Facing::DOWN ? tex + 16 : tex;
}

bool EndPortalFrameTile::isSolidRender()
{
	return false;
}

Tile::Shape EndPortalFrameTile::getRenderShape()
{
	return SHAPE_END_PORTAL_FRAME;
}

void EndPortalFrameTile::updateDefaultShape()
{
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.8125f, 1.0f);
}

void EndPortalFrameTile::addAABBs(Level &level, int_t x, int_t y, int_t z, AABB &bb, std::vector<AABB *> &aabbList)
{
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.8125f, 1.0f);
	Tile::addAABBs(level, x, y, z, bb, aabbList);
	if (hasEye(level.getData(x, y, z)))
	{
		setShape(0.3125f, 0.8125f, 0.3125f, 0.6875f, 1.0f, 0.6875f);
		Tile::addAABBs(level, x, y, z, bb, aabbList);
	}
	updateDefaultShape();
}

int_t EndPortalFrameTile::getResourceCount(Random &random)
{
	(void)random;
	return 0;
}

void EndPortalFrameTile::setPlacedBy(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	int_t dir = ((Mth::floor(player.yRot * 4.0f / 360.0f + 0.5) & 3) + 2) % 4;
	level.setData(x, y, z, dir);
}
