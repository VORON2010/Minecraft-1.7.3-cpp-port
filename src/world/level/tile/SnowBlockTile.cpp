#include "world/level/tile/SnowBlockTile.h"

#include "world/level/Level.h"
#include "world/item/Item.h"
#include "world/item/Items.h"
#include "java/Random.h"

SnowBlockTile::SnowBlockTile(int_t id, int_t tex, const Material &material) : Tile(id, tex, material)
{
	setTicking(true);
}

int_t SnowBlockTile::getResource(int_t data, Random &random)
{
	return Items::snowball->getShiftedIndex();
}

int_t SnowBlockTile::getResourceCount(Random &random)
{
	return 4;
}

void SnowBlockTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	if (level.getBrightness(LightLayer::Block, x, y, z) > 11)
	{
		spawnResources(level, x, y, z, level.getData(x, y, z));
		level.setTile(x, y, z, 0);
	}
}