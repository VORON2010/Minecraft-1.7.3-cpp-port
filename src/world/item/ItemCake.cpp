#include "world/item/ItemCake.h"

#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"
#include "world/level/tile/CakeTile.h"
#include "world/level/tile/SnowTile.h"
#include "world/level/tile/Tile.h"

ItemCake::ItemCake(int_t baseId, CakeTile &cakeTile) : Item(baseId), cakeTile(cakeTile)
{
	setMaxStackSize(1);
}

bool ItemCake::useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const
{
	(void)player;
	if (level.getTile(x, y, z) == Tile::snow.id)
	{
		face = Facing::UP;
	}
	else
	{
		if (face == Facing::DOWN) y--;
		if (face == Facing::UP) y++;
		if (face == Facing::NORTH) z--;
		if (face == Facing::SOUTH) z++;
		if (face == Facing::WEST) x--;
		if (face == Facing::EAST) x++;
	}

	if (stack.stackSize == 0)
		return false;

	if (!level.isEmptyTile(x, y, z) && level.getTile(x, y, z) != Tile::snow.id)
		return false;

	if (!cakeTile.mayPlace(level, x, y, z))
		return false;

	if (!level.setTileAndData(x, y, z, cakeTile.id, 0))
		return false;

	if (level.getTile(x, y, z) == cakeTile.id)
	{
		Tile::tiles[cakeTile.id]->onPlace(level, x, y, z);
		Tile::tiles[cakeTile.id]->setPlacedBy(level, x, y, z, player);
		stack.stackSize--;
	}
	return true;
}
