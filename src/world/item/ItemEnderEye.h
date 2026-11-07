#pragma once

#include "world/item/Item.h"

// 1.0 EnderEyeItem: fills portal frames, or flies toward the stronghold
class ItemEnderEye : public Item
{
public:
	explicit ItemEnderEye(int_t id);
	bool useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const override;
	void use(ItemInstance &stack, Level &level, Player &player) const override;
};
