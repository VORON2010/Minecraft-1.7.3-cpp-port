#pragma once

#include "world/item/Item.h"

// 1.5.2 FireChargeItem (385): lights a fire where it is used
class FireChargeItem : public Item
{
public:
	using Item::Item;
	bool useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const override;
};
