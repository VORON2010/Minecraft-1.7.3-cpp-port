#include "world/level/tile/FlowerTile.h"

#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/tile/GrassTile.h"
#include "world/level/tile/DirtTile.h"
#include "world/level/tile/FarmlandTile.h"

FlowerTile::FlowerTile(int_t id, int_t tex) : Tile(id, tex, Material::plants())
{
	setTicking(true);
	updateDefaultShape();
	updateCachedProperties();
}

bool FlowerTile::isCubeShaped()
{
	return false;
}

Tile::Shape FlowerTile::getRenderShape()
{
	return SHAPE_CROSS_TEXTURE;
}

AABB *FlowerTile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	return nullptr;
}

bool FlowerTile::isSolidRender()
{
	return false;
}

void FlowerTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	if (!canStay(level, x, y, z))
		level.setTile(x, y, z, 0);
}

void FlowerTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	if (!canStay(level, x, y, z))
		level.setTile(x, y, z, 0);
}

void FlowerTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	if (!canStay(level, x, y, z))
		level.setTile(x, y, z, 0);
}

void FlowerTile::updateDefaultShape()
{
	float radius = 0.2f;
	setShape(0.5f - radius, 0.0f, 0.5f - radius, 0.5f + radius, radius * 3.0f, 0.5f + radius);
}

bool FlowerTile::canSurviveOn(int_t belowTile) const
{
	return belowTile == Tile::grass.id || belowTile == Tile::dirt.id || belowTile == Tile::farmland.id;
}

bool FlowerTile::canStay(Level &level, int_t x, int_t y, int_t z)
{
	return (level.getRawBrightness(x, y, z) >= 8 || level.canSeeSky(x, y, z)) && canSurviveOn(level.getTile(x, y - 1, z));
}
