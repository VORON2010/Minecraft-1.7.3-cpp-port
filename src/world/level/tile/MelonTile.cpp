#include "world/level/tile/MelonTile.h"

#include "world/item/Item.h"
#include "world/item/Items.h"

MelonTile::MelonTile(int_t id) : Tile(id, 136, Material::pumpkin())
{
	updateCachedProperties();
}

int_t MelonTile::getTexture(Facing face, int_t data)
{
	(void)data;
	return getTexture(face);
}

int_t MelonTile::getTexture(Facing face)
{
	return (face == Facing::UP || face == Facing::DOWN) ? 137 : 136;
}

int_t MelonTile::getResource(int_t data, Random &random)
{
	(void)data;
	(void)random;
	return Items::melon->getShiftedIndex();
}

int_t MelonTile::getResourceCount(Random &random)
{
	return 3 + random.nextInt(5);
}

int_t MelonTile::getResourceCountForLootBonus(int_t bonus, Random &random)
{
	int_t n = getResourceCount(random) + random.nextInt(1 + bonus);
	return n > 9 ? 9 : n;
}
