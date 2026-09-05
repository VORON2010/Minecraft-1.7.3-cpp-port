#include "world/item/ItemSeeds.h"

#include "world/item/ItemInstance.h"
#include "world/level/Level.h"

ItemSeeds::ItemSeeds(int_t baseId, int_t resultTileId) : Item(baseId), resultTileId(resultTileId)
{
}

bool ItemSeeds::useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const
{
	(void)player;
	if (face != Facing::UP)
		return false;
	if (level.getTile(x, y, z) != 60)
		return false;
	if (!level.isEmptyTile(x, y + 1, z))
		return false;
	if (!level.setTile(x, y + 1, z, resultTileId))
		return false;

	stack.stackSize--;
	return true;
}
