#include "world/item/ItemFood.h"

#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"

ItemFood::ItemFood(int_t baseId, int_t healAmount, float saturationModifier, bool wolfsFavoriteMeat)
	: Item(baseId), healAmount(healAmount), saturationModifier(saturationModifier), wolfsFavoriteMeat(wolfsFavoriteMeat)
{
	setMaxStackSize(64);
}

ItemFood &ItemFood::setEffect(int_t id, int_t seconds, int_t amplifier, float chance)
{
	effectId = id;
	effectDuration = seconds;
	effectAmplifier = amplifier;
	effectChance = chance;
	return *this;
}

void ItemFood::use(ItemInstance &stack, Level &level, Player &player) const
{
	(void)level;
	if (stack.isEmpty())
		return;
	if (player.canEat(alwaysEdible))
		player.startUsingItem(stack, getUseDuration(stack));
}

void ItemFood::useTimeDepleted(ItemInstance &stack, Level &level, Player &player) const
{
	if (stack.isEmpty())
		return;
	stack.stackSize--;
	player.feed(healAmount, saturationModifier);
	level.playSoundAtEntity(player, u"random.burp", 0.5f, level.random.nextFloat() * 0.1f + 0.9f);

	if (!level.isOnline && effectId > 0 && level.random.nextFloat() < effectChance)
		player.addEffect(MobEffectInstance(effectId, effectDuration * 20, effectAmplifier));
}
