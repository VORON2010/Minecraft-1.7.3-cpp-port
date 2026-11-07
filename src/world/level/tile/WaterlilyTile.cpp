#include "world/level/tile/WaterlilyTile.h"

#include "world/level/Level.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/level/tile/LiquidTile.h"

WaterlilyTile::WaterlilyTile(int_t id, int_t tex) : FlowerTile(id, tex)
{
	updateDefaultShape();
}

void WaterlilyTile::updateDefaultShape()
{
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.015625f, 1.0f);
}

AABB *WaterlilyTile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	(void)level;
	return AABB::newTemp(x + xx0, y + yy0, z + zz0, x + xx1, y + yy1, z + zz1);
}

int_t WaterlilyTile::getColor(LevelSource &level, int_t x, int_t y, int_t z)
{
	(void)level; (void)x; (void)y; (void)z;
	return 2129968;
}

int_t WaterlilyTile::getItemColor(int_t data)
{
	(void)data;
	return 2129968;
}

bool WaterlilyTile::canSurviveOn(int_t belowTile) const
{
	return belowTile == Tile::calmWater.id;
}

bool WaterlilyTile::canStay(Level &level, int_t x, int_t y, int_t z)
{
	if (y < 0 || y >= Level::DEPTH)
		return false;
	return &level.getMaterial(x, y - 1, z) == static_cast<const Material *>(&Material::water) && level.getData(x, y - 1, z) == 0;
}
