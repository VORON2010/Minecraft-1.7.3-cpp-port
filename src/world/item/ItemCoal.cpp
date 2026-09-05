#include "world/item/ItemCoal.h"

#include "world/item/ItemInstance.h"

ItemCoal::ItemCoal(int_t id) : Item(id)
{
	setHasSubtypes(true);
	setMaxDamage(0);
}

jstring ItemCoal::getDescriptionId(const ItemInstance &stack) const
{
	return stack.itemDamage == 1 ? jstring(u"item.charcoal") : Item::getDescriptionId(stack);
}
