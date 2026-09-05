#pragma once

#include "world/item/Item.h"

class ItemFlintAndSteel : public Item
{
public:
	explicit ItemFlintAndSteel(int_t baseId);

	bool useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const override;
};
