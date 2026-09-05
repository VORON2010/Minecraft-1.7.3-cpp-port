#pragma once

#include "world/item/ItemTool.h"

class ItemPickaxe : public ItemTool
{
public:
	ItemPickaxe(int_t baseId, ToolMaterialType material);

	bool canDestroySpecial(const ItemInstance &stack, Tile &tile) const override;
};