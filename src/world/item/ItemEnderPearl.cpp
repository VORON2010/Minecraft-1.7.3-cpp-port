#include "world/item/ItemEnderPearl.h"

#include "world/entity/player/Player.h"
#include "world/entity/projectile/ThrownEnderpearl.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"

ItemEnderPearl::ItemEnderPearl(int_t id) : Item(id)
{
	setMaxStackSize(16);
}

void ItemEnderPearl::use(ItemInstance &stack, Level &level, Player &player) const
{
	// 1.0: pearls do nothing in creative
	if (player.abilities.instabuild || stack.stackSize <= 0)
		return;
	stack.stackSize--;
	level.playSoundAtEntity(player, u"random.bow", 0.5f, 0.4f / (itemRandom.nextFloat() * 0.4f + 0.8f));
	if (!level.isOnline)
		level.addEntity(std::make_shared<ThrownEnderpearl>(level, player));
}
