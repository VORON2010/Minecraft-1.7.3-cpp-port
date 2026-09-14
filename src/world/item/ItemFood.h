#pragma once

#include "world/item/Item.h"

class ItemFood : public Item
{
private:
	int_t healAmount = 0;
	float saturationModifier = 0.6f;
	bool wolfsFavoriteMeat = false;

public:
	ItemFood(int_t baseId, int_t healAmount, float saturationModifier, bool wolfsFavoriteMeat);
	void use(ItemInstance &stack, Level &level, Player &player) const override;

	int_t getHealAmount() const { return healAmount; }
	float getSaturationModifier() const { return saturationModifier; }
	bool isWolfsFavoriteMeat() const { return wolfsFavoriteMeat; }
};
