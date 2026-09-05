#pragma once

#include "world/item/Item.h"

class ItemBow : public Item
{
public:
	explicit ItemBow(int_t id);
	void use(ItemInstance &stack, Level &level, Player &player) const override;
};
