#pragma once

#include "world/item/Item.h"

class ItemBucket : public Item
{
private:
	int_t containedBlockId = 0;

public:
	ItemBucket(int_t baseId, int_t containedBlockId);
	void use(ItemInstance &stack, Level &level, Player &player) const override;
};
