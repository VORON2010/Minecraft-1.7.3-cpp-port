#include "world/item/ItemEgg.h"

#include "world/entity/player/Player.h"
#include "world/entity/projectile/EntityThrownEgg.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"

ItemEgg::ItemEgg(int_t id) : Item(id)
{
	setMaxStackSize(16);
}

void ItemEgg::use(ItemInstance &stack, Level &level, Player &player) const
{
	if (stack.stackSize <= 0)
		return;
	stack.stackSize--;
	level.playSoundAtEntity(player, u"random.bow", 0.5f, 0.4f / (itemRandom.nextFloat() * 0.4f + 0.8f));
	if (!level.isOnline)
		level.addEntity(std::make_shared<EntityThrownEgg>(level, player));
}
