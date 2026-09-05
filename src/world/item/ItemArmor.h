#pragma once

#include "world/item/Item.h"

class ItemArmor : public Item
{
public:
	static const int_t damageReduceAmountArray[];
	static const int_t maxDamageArray[];

	const int_t armorLevel;
	const int_t armorType;
	const int_t damageReduceAmount;
	const int_t renderIndex;

	ItemArmor(int_t id, int_t armorLevel, int_t renderIndex, int_t armorType);
};