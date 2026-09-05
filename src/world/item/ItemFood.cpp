#include "world/item/ItemFood.h"

#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"

ItemFood::ItemFood(int_t baseId, int_t healAmount, bool wolfsFavoriteMeat)
	: Item(baseId), healAmount(healAmount), wolfsFavoriteMeat(wolfsFavoriteMeat)
{
	setMaxStackSize(1);
}

void ItemFood::use(ItemInstance &stack, Level &level, Player &player) const
{
	(void)level;
	if (stack.isEmpty())
		return;
	stack.stackSize--;
	player.heal(healAmount);
}
