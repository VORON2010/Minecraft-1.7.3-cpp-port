#pragma once

#include "world/item/Item.h"

class ItemPainting : public Item
{
public:
	explicit ItemPainting(int_t id) : Item(id) {}
	bool useOn(ItemInstance &stack, Player &player, Level &level,
		int_t x, int_t y, int_t z, Facing face) const override;
};
