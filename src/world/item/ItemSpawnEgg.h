#pragma once

#include "world/item/Item.h"

class ItemSpawnEgg : public Item
{
private:
	int_t spawnedEntityId;

public:
	ItemSpawnEgg(int_t baseId, int_t entityId);
	bool useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const override;
};
