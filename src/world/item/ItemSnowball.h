#pragma once

#include "world/item/Item.h"

class ItemSnowball : public Item
{
public:
	explicit ItemSnowball(int_t id);
	void use(ItemInstance &stack, Level &level, Player &player) const override;
};
