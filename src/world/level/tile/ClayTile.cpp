#include "world/level/tile/ClayTile.h"
#include "world/item/Item.h"
#include "world/item/Items.h"

ClayTile::ClayTile(int_t id, int_t tex) : Tile(id, tex, Material::clay)
{
}

int_t ClayTile::getResource(int_t data, Random &random)
{
	return Items::clayItem->getShiftedIndex();
}

int_t ClayTile::getResourceCount(Random &random)
{
	return 4;
}
