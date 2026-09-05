#pragma once

#include "world/item/ItemTool.h"

class ItemSpade : public ItemTool
{
public:
	ItemSpade(int_t baseId, ToolMaterialType material);
	bool canDestroySpecial(const ItemInstance &stack, Tile &tile) const override;

};