#include "world/item/ItemMinecart.h"

#include "world/entity/item/EntityMinecart.h"
#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"
#include "world/level/tile/RailTile.h"

ItemMinecart::ItemMinecart(int_t baseId, int_t minecartType) : Item(baseId), minecartType(minecartType)
{
	setMaxStackSize(1);
}

bool ItemMinecart::useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const
{
	(void)player;
	(void)face;
	if (!RailTile::isRail(level.getTile(x, y, z)))
		return false;

	if (!level.isOnline)
		level.addEntity(std::make_shared<EntityMinecart>(level, x + 0.5, y + 0.5, z + 0.5, minecartType));

	stack.stackSize--;
	return true;
}
