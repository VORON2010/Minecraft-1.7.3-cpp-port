#pragma once

#include "world/item/ItemFood.h"

class ItemSoup : public ItemFood
{
public:
	ItemSoup(int_t baseId, int_t healAmount);
	void useTimeDepleted(ItemInstance &stack, Level &level, Player &player) const override;
};
