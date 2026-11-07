#include "client/gui/ContainerScreen.h"
#include "world/item/enchantment/Enchantment.h"

#include "client/Minecraft.h"
#include "client/Options.h"
#include "client/locale/Language.h"
#include "world/entity/player/Player.h"
#include "world/item/Item.h"
#include "world/item/ItemDye.h"
#include "world/item/ItemInstance.h"
#include "world/level/tile/ClothTile.h"
#include "world/level/tile/Tile.h"

ContainerScreen::ContainerScreen(Minecraft &minecraft) : Screen(minecraft)
{
}

bool ContainerScreen::shouldClose(bool alive, bool removed)
{
	return !alive || removed;
}


jstring ContainerScreen::getTooltipName(const ItemInstance &stack)
{
	if (!stack.customName.empty())
		return u"\u00a7o" + stack.customName;
	jstring key;
	Item *item = stack.getItem();
	if (item != nullptr)
	{
		jstring custom = item->getHoverName(stack);
		if (!custom.empty())
			return custom;
		key = item->getDescriptionId(stack);
	}
	else if (stack.itemID > 0 && stack.itemID < static_cast<int_t>(Tile::tiles.size()) && Tile::tiles[stack.itemID] != nullptr)
	{
		
		key = Tile::tiles[stack.itemID]->descriptionId;
		if (stack.itemID == 145)
		{
			static const char16_t *const anvilNames[] = {u".intact", u".slightlyDamaged", u".veryDamaged"};
			int_t d = stack.itemDamage;
			key += anvilNames[d >= 0 && d < 3 ? d : 0];
		}
		if (stack.itemID == Tile::wool.id)
			key += u"." + ItemDye::getDyeColorName(~stack.itemDamage & 15);
	}

	jstring name = Language::getInstance().getElementName(key);
	size_t start = 0;
	while (start < name.size() && static_cast<uchar_t>(name[start]) <= u' ')
		start++;
	size_t end = name.size();
	while (end > start && static_cast<uchar_t>(name[end - 1]) <= u' ')
		end--;
	return name.substr(start, end - start);
}

std::vector<jstring> ContainerScreen::getTooltipLines(const ItemInstance &stack, const Options *options)
{
	std::vector<jstring> lines;
	jstring name = getTooltipName(stack);
	if (!name.empty())
		lines.push_back(stack.isEnchanted() ? u"\u00a7b" + name : (!stack.storedEnchantments.empty() ? u"\u00a7e" + name : name));
	for (const auto *list : {&stack.enchantments, &stack.storedEnchantments})
		for (const auto &e : *list)
			if (e.first >= 0 && e.first < 256 && Enchantment::enchantments[e.first] != nullptr)
				lines.push_back(u"\u00a77" + Enchantment::enchantments[e.first]->getFullname(e.second));
	if (Item *item = stack.getItem())
		item->appendHoverText(stack, lines);

	if (options != nullptr && options->advancedItemTooltips)
	{
		Item *item = stack.getItem();
		if (item != nullptr && item->getMaxDamage() > 0)
		{
			int_t remaining = item->getMaxDamage() - stack.itemDamage;
			lines.push_back(u"Durability: " + String::toString(remaining) + u" / " + String::toString(item->getMaxDamage()));
		}
		jstring idStr = u"#" + String::toString(stack.itemID);
		if (stack.itemDamage != 0)
			idStr += u"/" + String::toString(stack.itemDamage);
		lines.push_back(idStr);
	}
	return lines;
}

void ContainerScreen::tick()
{
	Screen::tick();
	if (minecraft.player != nullptr &&
		shouldClose(minecraft.player->isAlive(), minecraft.player->removed))
	{
		minecraft.player->closeContainer();
	}
}
