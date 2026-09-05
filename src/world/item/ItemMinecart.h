#pragma once

#include "world/item/Item.h"

class ItemMinecart : public Item
{
private:
	int_t minecartType = 0;

public:
	ItemMinecart(int_t baseId, int_t minecartType);
	bool useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const override;
};
