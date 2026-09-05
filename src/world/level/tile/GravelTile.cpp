#include "world/level/tile/GravelTile.h"

#include "world/item/Item.h"
#include "world/item/Items.h"

GravelTile::GravelTile(int_t id, int_t tex) : SandTile(id, tex)
{
}

int_t GravelTile::getResource(int_t data, Random &random)
{
	(void)data;
	if (Items::flint != nullptr && random.nextInt(10) == 0)
		return Items::flint->getShiftedIndex();
	return id;
}
