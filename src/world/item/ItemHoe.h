#pragma once

#include "world/item/Item.h"
#include "world/item/ItemTool.h"

class ItemHoe : public Item
{
public:
	ItemHoe(int_t baseId, ToolMaterialType material);

	bool useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const override;
	bool isFull3D() const override { return true; }
};
