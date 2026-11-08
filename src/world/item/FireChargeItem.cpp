#include "world/item/FireChargeItem.h"

#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"
#include "world/level/tile/FireTile.h"
#include "world/level/tile/Tile.h"

bool FireChargeItem::useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const
{
	(void)player;
	if (level.isOnline)
		return true;
	if (face == Facing::DOWN) y--;
	if (face == Facing::UP) y++;
	if (face == Facing::NORTH) z--;
	if (face == Facing::SOUTH) z++;
	if (face == Facing::WEST) x--;
	if (face == Facing::EAST) x++;
	if (level.getTile(x, y, z) == 0)
	{
		level.playSoundEffect(x + 0.5, y + 0.5, z + 0.5, u"fire.ignite", 1.0f, itemRandom.nextFloat() * 0.4f + 0.8f);
		level.setTile(x, y, z, Tile::fire.id);
	}
	if (!player.abilities.instabuild)
		stack.stackSize--;
	return true;
}
