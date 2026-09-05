#pragma once

#include "world/item/Item.h"

class ItemBoat : public Item
{
public:
	ItemBoat(int_t baseId);
	void use(ItemInstance &stack, Level &level, Player &player) const override;
};
