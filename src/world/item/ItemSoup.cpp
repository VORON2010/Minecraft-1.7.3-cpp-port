#include "world/item/ItemSoup.h"

#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/level/Level.h"

ItemSoup::ItemSoup(int_t baseId, int_t healAmount)
	: ItemFood(baseId, healAmount, 0.6f, false)
{
	setMaxStackSize(1);
}

void ItemSoup::useTimeDepleted(ItemInstance &stack, Level &level, Player &player) const
{
	ItemFood::useTimeDepleted(stack, level, player);
	if (stack.isEmpty())
		stack = ItemInstance(Items::bowlEmpty->getShiftedIndex(), 1, 0);
}
