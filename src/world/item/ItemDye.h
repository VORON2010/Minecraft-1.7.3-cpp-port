#pragma once

#include "world/item/Item.h"

class ItemDye : public Item
{
public:
	ItemDye(int_t baseId);

	int_t getIcon(const ItemInstance &stack) const override;
	jstring getDescriptionId(const ItemInstance &stack) const override;
	
	static const jstring &getDyeColorName(int_t colour);
	void saddleEntity(ItemInstance &stack, Mob &target) const override;
};