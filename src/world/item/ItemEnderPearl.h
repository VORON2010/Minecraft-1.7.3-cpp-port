#pragma once

#include "world/item/Item.h"

class ItemEnderPearl : public Item
{
public:
	explicit ItemEnderPearl(int_t id);
	void use(ItemInstance &stack, Level &level, Player &player) const override;
};
