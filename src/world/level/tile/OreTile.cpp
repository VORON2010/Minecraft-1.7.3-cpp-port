#include "world/level/tile/OreTile.h"
#include "world/item/Item.h"
#include "world/item/Items.h"
#include "world/level/tile/Tile.h"

OreTile::OreTile(int_t id, int_t tex) : Tile(id, tex, Material::stone)
{
}

int_t OreTile::getResource(int_t data, Random &random)
{
	if (id == Tile::coalOre.id)
		return Items::coal->getShiftedIndex();
	if (id == Tile::diamondOre.id)
		return Items::diamond->getShiftedIndex();
	if (id == Tile::lapisOre.id)
		return Items::dyePowder->getShiftedIndex();
	return id; 
}

int_t OreTile::getResourceCount(Random &random)
{
	if (id == Tile::lapisOre.id)
		return 4 + random.nextInt(5);
	return 1;
}

int_t OreTile::getSpawnResourcesAuxValue(int_t data)
{
	if (id == Tile::lapisOre.id)
		return 4; 
	return 0;
}
int_t OreTile::getResourceCountForLootBonus(int_t bonus, Random &random)
{
	if (bonus > 0 && id != getResource(0, random))
	{
		int_t extra = random.nextInt(bonus + 2) - 1;
		if (extra < 0)
			extra = 0;
		return getResourceCount(random) * (extra + 1);
	}
	return getResourceCount(random);
}
