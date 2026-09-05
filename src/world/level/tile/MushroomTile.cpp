#include "world/level/tile/MushroomTile.h"

#include "world/level/Level.h"
#include "world/level/tile/Tile.h"

MushroomTile::MushroomTile(int_t id, int_t tex) : FlowerTile(id, tex)
{
	updateDefaultShape();
}

void MushroomTile::updateDefaultShape()
{
	float radius = 0.2f;
	setShape(0.5f - radius, 0.0f, 0.5f - radius, 0.5f + radius, radius * 2.0f, 0.5f + radius);
}

bool MushroomTile::canSurviveOn(int_t belowTile) const
{
	return belowTile != 0 && Tile::solid[belowTile];
}

bool MushroomTile::canStay(Level &level, int_t x, int_t y, int_t z)
{
	return y >= 0 && y < Level::DEPTH && level.getRawBrightness(x, y, z) < 13 && canSurviveOn(level.getTile(x, y - 1, z));
}
