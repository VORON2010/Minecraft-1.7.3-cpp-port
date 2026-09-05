#include "world/item/ItemRedStone.h"

#include "world/item/ItemInstance.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/RedStoneDustTile.h"

ItemRedStone::ItemRedStone(int_t baseId) : Item(baseId)
{
}

bool ItemRedStone::useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const
{
	(void)player;

	if (face == Facing::DOWN)
		y--;
	if (face == Facing::UP)
		y++;
	if (face == Facing::NORTH)
		z--;
	if (face == Facing::SOUTH)
		z++;
	if (face == Facing::WEST)
		x--;
	if (face == Facing::EAST)
		x++;

	if (!level.isEmptyTile(x, y, z))
		return false;

	if (Tile::redstoneWire.mayPlace(level, x, y, z))
	{
		stack.stackSize--;
		level.setTile(x, y, z, Tile::redstoneWire.id);
	}

	return true;
}
