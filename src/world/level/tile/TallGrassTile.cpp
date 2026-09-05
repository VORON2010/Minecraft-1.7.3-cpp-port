#include "world/level/tile/TallGrassTile.h"

#include "world/item/Item.h"
#include "world/item/Items.h"
#include "world/level/LevelSource.h"
#include "world/level/GrassColor.h"

TallGrassTile::TallGrassTile(int_t id, int_t tex) : FlowerTile(id, tex)
{
	updateDefaultShape();
}

int_t TallGrassTile::getTexture(Facing face, int_t data)
{
	if (data == 1)
		return tex;
	if (data == 2)
		return tex + 17;
	if (data == 0)
		return tex + 16;
	return tex;
}

int_t TallGrassTile::getColor(LevelSource &level, int_t x, int_t y, int_t z)
{
	int_t data = level.getData(x, y, z);
	if (data == 0)
		return 0xFFFFFF;

	level.getBiomeSource().getBiomeBlock(x, z, 1, 1);

	return GrassColor::get(level.getBiomeSource().temperatures[0], level.getBiomeSource().downfalls[0]);
}


int_t TallGrassTile::getItemColor(int_t data)
{
	(void)data;
	return 0xFFFFFF;
}


int_t TallGrassTile::getResource(int_t data, Random &random)
{
	return random.nextInt(8) == 0 ? Items::seeds->getShiftedIndex() : -1;
}

void TallGrassTile::updateDefaultShape()
{
	float radius = 0.4f;
	setShape(0.5f - radius, 0.0f, 0.5f - radius, 0.5f + radius, 0.8f, 0.5f + radius);
}