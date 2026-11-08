#include "world/item/ExperienceBottleItem.h"

#include "world/entity/player/Player.h"
#include "world/entity/projectile/ThrownExpBottle.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"

void ExperienceBottleItem::use(ItemInstance &stack, Level &level, Player &player) const
{
	if (!player.abilities.instabuild)
		stack.stackSize--;
	level.playSoundAtEntity(player, u"random.bow", 0.5f, 0.4f / (itemRandom.nextFloat() * 0.4f + 0.8f));
	if (!level.isOnline)
		level.addEntity(std::make_shared<ThrownExpBottle>(level, player));
}
