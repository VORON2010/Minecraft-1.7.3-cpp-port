#pragma once

#include "world/item/Item.h"

class ItemFishingRod : public Item
{
public:
	explicit ItemFishingRod(int_t id);

	bool isFull3D() const override { return true; }
	bool shouldRotateAroundWhenRendering() const override { return true; }
	void use(ItemInstance &stack, Level &level, Player &player) const override;
};
