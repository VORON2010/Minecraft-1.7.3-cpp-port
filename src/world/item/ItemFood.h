#pragma once

#include "world/item/Item.h"

class ItemFood : public Item
{
private:
	int_t healAmount = 0;
	bool wolfsFavoriteMeat = false;

public:
	ItemFood(int_t baseId, int_t healAmount, bool wolfsFavoriteMeat);
	void use(ItemInstance &stack, Level &level, Player &player) const override;

	int_t getHealAmount() const { return healAmount; }
	bool isWolfsFavoriteMeat() const { return wolfsFavoriteMeat; }
};
