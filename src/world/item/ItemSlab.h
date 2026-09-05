#pragma once

#include "world/item/Item.h"

class ItemSlab : public Item
{
public:
	ItemSlab(int_t baseId);

	int_t getIcon(const ItemInstance &stack) const override;
	jstring getDescriptionId(const ItemInstance &stack) const override;
	bool useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const override;
};
