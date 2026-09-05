#include "world/item/ItemArmor.h"

const int_t ItemArmor::damageReduceAmountArray[] = {3, 8, 6, 3};
const int_t ItemArmor::maxDamageArray[] = {11, 16, 15, 13};

ItemArmor::ItemArmor(int_t id, int_t armorLevel, int_t renderIndex, int_t armorType)
	: Item(id), armorLevel(armorLevel), armorType(armorType), renderIndex(renderIndex),
	  damageReduceAmount(damageReduceAmountArray[armorType])
{
	setMaxStackSize(1);
	setMaxDamage(maxDamageArray[armorType] * 3 << armorLevel);
}