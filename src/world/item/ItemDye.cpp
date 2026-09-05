#include "world/item/ItemDye.h"

#include "world/entity/animal/Sheep.h"
#include "world/item/ItemInstance.h"

namespace
{
	const jstring dyeColors[] = {
		u"black", u"red", u"green", u"brown",
		u"blue", u"purple", u"cyan", u"silver",
		u"gray", u"pink", u"lime", u"yellow",
		u"lightBlue", u"magenta", u"orange", u"white"
	};
}

ItemDye::ItemDye(int_t baseId) : Item(baseId)
{
	setHasSubtypes(true);
	setMaxDamage(0);
	setMaxStackSize(64);
}

int_t ItemDye::getIcon(const ItemInstance &stack) const
{
	return iconIndex + (stack.itemDamage % 8) * 16 + (stack.itemDamage / 8);
}

const jstring &ItemDye::getDyeColorName(int_t colour)
{
	return dyeColors[colour & 15];
}

jstring ItemDye::getDescriptionId(const ItemInstance &stack) const
{
	int_t dye = stack.itemDamage & 15;
	return u"item.dyePowder." + dyeColors[dye];
}

void ItemDye::saddleEntity(ItemInstance &stack, Mob &target) const
{
	Sheep *sheep = dynamic_cast<Sheep *>(&target);
	if (sheep != nullptr)
	{
		int_t color = ~stack.itemDamage & 15;
		if (!sheep->isSheared() && sheep->getFleeceColor() != color)
		{
			sheep->setFleeceColor(color);
			stack.stackSize--;
		}
	}
}