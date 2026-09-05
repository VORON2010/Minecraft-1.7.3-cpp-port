#include "world/level/tile/GlowStoneTile.h"

#include "world/item/Item.h"
#include "world/item/Items.h"
#include "java/Random.h"

GlowStoneTile::GlowStoneTile(int_t id, int_t tex, const Material &material) : Tile(id, tex, material)
{
}

int_t GlowStoneTile::getResource(int_t data, Random &random)
{
	return Items::glowstoneDust->getShiftedIndex();
}

int_t GlowStoneTile::getResourceCount(Random &random)
{
	return 2 + random.nextInt(3);
}