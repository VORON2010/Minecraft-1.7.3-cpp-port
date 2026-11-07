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
int_t GlowStoneTile::getResourceCountForLootBonus(int_t bonus, Random &random)
{
	int_t n = getResourceCount(random) + random.nextInt(bonus + 1);
	return n < 1 ? 1 : (n > 4 ? 4 : n);
}
