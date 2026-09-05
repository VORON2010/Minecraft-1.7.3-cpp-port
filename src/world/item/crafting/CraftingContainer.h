#pragma once

#include "java/Type.h"
#include "world/item/ItemInstance.h"

class CraftingContainer
{
public:
	virtual ~CraftingContainer() {}

	virtual ItemInstance getItem(int_t x, int_t y) const = 0;
};
