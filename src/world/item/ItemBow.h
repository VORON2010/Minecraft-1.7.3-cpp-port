#pragma once

#include "world/item/Item.h"

class ItemBow : public Item
{
public:
	explicit ItemBow(int_t id);
	void use(ItemInstance &stack, Level &level, Player &player) const override;
	void releaseUsing(ItemInstance &stack, Level &level, Player &player, int_t remainingTime) const override;
	UseAnim getUseAnimation(const ItemInstance &stack) const override { (void)stack; return UseAnim::BOW; }
	int_t getUseDuration(const ItemInstance &stack) const override { (void)stack; return 72000; }
};
