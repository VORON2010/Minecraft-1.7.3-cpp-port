#include "world/item/ItemArmor.h"
#include "world/item/Items.h"
#include "world/item/ItemInstance.h"

const int_t ItemArmor::damageReduceAmountArray[] = {3, 8, 6, 3};
const int_t ItemArmor::maxDamageArray[] = {11, 16, 15, 13};

ItemArmor::ItemArmor(int_t id, int_t armorLevel, int_t renderIndex, int_t armorType)
	: Item(id), armorLevel(armorLevel), armorType(armorType), renderIndex(renderIndex),
	  damageReduceAmount(damageReduceAmountArray[armorType])
{
	setMaxStackSize(1);
	setMaxDamage(maxDamageArray[armorType] * 3 << armorLevel);
}
bool ItemArmor::isValidRepairItem(const ItemInstance &stack, const ItemInstance &material) const
{
	// 1.4.7 ArmorItem.Tier.getRepairIngredient: leather, iron (chain too), iron, diamond, gold
	(void)stack;
	int_t wanted = 0;
	switch (renderIndex)
	{
	case 0: wanted = Items::leather->getShiftedIndex(); break;
	case 1: wanted = Items::ingotIron->getShiftedIndex(); break;
	case 2: wanted = Items::ingotIron->getShiftedIndex(); break;
	case 3: wanted = Items::diamond->getShiftedIndex(); break;
	case 4: wanted = Items::ingotGold->getShiftedIndex(); break;
	}
	return material.itemID == wanted;
}
