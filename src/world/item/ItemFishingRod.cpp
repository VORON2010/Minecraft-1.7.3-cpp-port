#include "world/item/ItemFishingRod.h"

#include <memory>

#include "world/entity/player/Player.h"
#include "world/entity/projectile/EntityFish.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"

ItemFishingRod::ItemFishingRod(int_t id) : Item(id)
{
	setMaxDamage(64);
	setMaxStackSize(1);
}

void ItemFishingRod::use(ItemInstance &stack, Level &level, Player &player) const
{
	if (player.fishEntity != nullptr)
	{
		std::shared_ptr<EntityFish> fish = player.fishEntity;
		int_t damage = fish->catchFish();
		stack.damageItem(damage, player);
		player.swing();
	}
	else
	{
		level.playSoundAtEntity(player, u"random.bow", 0.5f,
			0.4f / (itemRandom.nextFloat() * 0.4f + 0.8f));
		if (!level.isOnline)
		{
			auto fish = std::make_shared<EntityFish>(level, player);
			player.fishEntity = fish;
			level.addEntity(fish);
		}
		player.swing();
	}
}
