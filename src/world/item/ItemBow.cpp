#include "world/item/ItemBow.h"

#include "world/entity/player/Player.h"
#include "world/entity/projectile/EntityArrow.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/level/Level.h"

ItemBow::ItemBow(int_t id) : Item(id)
{
	setMaxStackSize(1);
	setMaxDamage(384);
}

void ItemBow::use(ItemInstance &stack, Level &level, Player &player) const
{
	(void)level;
	if (player.abilities.instabuild || player.inventory.hasItem(Items::arrow->getShiftedIndex()))
		player.startUsingItem(stack, getUseDuration(stack));
}

void ItemBow::releaseUsing(ItemInstance &stack, Level &level, Player &player, int_t remainingTime) const
{
	bool creative = player.abilities.instabuild;
	if (!creative && !player.inventory.hasItem(Items::arrow->getShiftedIndex()))
		return;

	int_t ticks = getUseDuration(stack) - remainingTime;
	float power = static_cast<float>(ticks) / 20.0f;
	power = (power * power + power * 2.0f) / 3.0f;
	if (power < 0.1f)
		return;
	if (power > 1.0f)
		power = 1.0f;

	auto arrow = std::make_shared<EntityArrow>(level, player, power * 2.0f);
	if (power == 1.0f)
		arrow->critArrow = true;

	stack.damageItem(1, player);
	level.playSoundAtEntity(player, u"random.bow", 1.0f, 1.0f / (itemRandom.nextFloat() * 0.4f + 1.2f) + power * 0.5f);
	if (!creative)
		player.inventory.consumeItem(Items::arrow->getShiftedIndex());
	if (!level.isOnline)
		level.addEntity(arrow);
}
