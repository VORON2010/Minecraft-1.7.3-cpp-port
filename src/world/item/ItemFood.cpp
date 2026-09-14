#include "world/item/ItemFood.h"

#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"

ItemFood::ItemFood(int_t baseId, int_t healAmount, float saturationModifier, bool wolfsFavoriteMeat)
	: Item(baseId), healAmount(healAmount), saturationModifier(saturationModifier), wolfsFavoriteMeat(wolfsFavoriteMeat)
{
	setMaxStackSize(64);
}

void ItemFood::use(ItemInstance &stack, Level &level, Player &player) const
{
	(void)level;
	if (stack.isEmpty())
		return;
	if (player.foodLevel >= 20 && getShiftedIndex() != 322 && getShiftedIndex() != 367) // 322 is Golden Apple, 367 is Rotten Flesh
		return;
	stack.stackSize--;
	player.feed(healAmount, saturationModifier);
	
	if (getShiftedIndex() == 367 && itemRandom.nextFloat() < 0.8f) // 367 is Rotten Flesh
	{
		player.hungerEffectTimer = 600; // 30 seconds
	}
}
