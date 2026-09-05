#pragma once

#include "world/item/Item.h"

class ItemSign : public Item
{
public:
	ItemSign(int_t id);
	bool useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const override;
};