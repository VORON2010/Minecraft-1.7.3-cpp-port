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
	// 1.0 ArmorItem.Tier enchantability: cloth, chain, iron, diamond, gold
	int_t getEnchantmentValue() const override { static const int_t v[] = {15, 12, 9, 10, 25}; return renderIndex >= 0 && renderIndex < 5 ? v[renderIndex] : 0; }
};