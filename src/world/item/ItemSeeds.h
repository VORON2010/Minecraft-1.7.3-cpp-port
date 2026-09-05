#pragma once

#include "world/item/Item.h"

class ItemSeeds : public Item
{
public:
	ItemSeeds(int_t baseId, int_t resultTileId);

	bool useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const override;

private:
	int_t resultTileId = 0;
};
