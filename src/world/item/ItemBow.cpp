#include "world/item/ItemBow.h"

#include "world/entity/player/Player.h"
#include "world/entity/projectile/EntityArrow.h"
#include "world/item/Items.h"
#include "world/level/Level.h"

ItemBow::ItemBow(int_t id) : Item(id)
{
	setMaxStackSize(1);
}

void ItemBow::use(ItemInstance &stack, Level &level, Player &player) const
{
	(void)stack;
	if (!player.inventory.consumeItem(Items::arrow->getShiftedIndex()))
		return;
	level.playSoundAtEntity(player, u"random.bow", 1.0f, 1.0f / (itemRandom.nextFloat() * 0.4f + 0.8f));
	if (!level.isOnline)
		level.addEntity(std::make_shared<EntityArrow>(level, player));
}
