#pragma once

#include "world/item/Item.h"

// 1.5.2 ExperienceBottleItem (384)
class ExperienceBottleItem : public Item
{
public:
	using Item::Item;
	bool isFoil(const ItemInstance &stack) const override { (void)stack; return true; }
	void use(ItemInstance &stack, Level &level, Player &player) const override;
};
