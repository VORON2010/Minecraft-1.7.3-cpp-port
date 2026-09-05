#pragma once

#include "world/item/Item.h"

class CakeTile;

class ItemCake : public Item
{
public:
	ItemCake(int_t baseId, CakeTile &cakeTile);

	bool useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const override;

private:
	CakeTile &cakeTile;
};
