#pragma once

#include "world/item/Item.h"

class DoorTile;

class ItemDoor : public Item
{
private:
	DoorTile &doorTile;

public:
	ItemDoor(int_t baseId, DoorTile &doorTile);

	bool useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const override;
};