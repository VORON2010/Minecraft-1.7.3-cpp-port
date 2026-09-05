#include "world/item/ItemSoup.h"

#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/level/Level.h"

ItemSoup::ItemSoup(int_t baseId, int_t healAmount)
	: ItemFood(baseId, healAmount, false)
{
	setMaxStackSize(1);
}

void ItemSoup::use(ItemInstance &stack, Level &level, Player &player) const
{
	(void)level;
	if (stack.isEmpty())
		return;
	stack.stackSize--;
	player.heal(getHealAmount());
	if (stack.isEmpty())
		stack = ItemInstance(Items::bowlEmpty->getShiftedIndex(), 1, 0);
}
