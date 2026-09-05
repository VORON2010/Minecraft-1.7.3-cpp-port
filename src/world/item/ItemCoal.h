#pragma once

#include "world/item/Item.h"

class ItemCoal : public Item
{
public:
	explicit ItemCoal(int_t id);

	jstring getDescriptionId(const ItemInstance &stack) const override;
};
