#pragma once

#include "world/item/Item.h"

class ItemEgg : public Item
{
public:
	explicit ItemEgg(int_t id);
	void use(ItemInstance &stack, Level &level, Player &player) const override;
};
