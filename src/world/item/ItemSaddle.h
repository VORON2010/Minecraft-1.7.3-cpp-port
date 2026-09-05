#pragma once

#include "world/item/Item.h"

class ItemSaddle : public Item
{
public:
	explicit ItemSaddle(int_t id);
	void saddleEntity(ItemInstance &stack, Mob &target) const override;
};
