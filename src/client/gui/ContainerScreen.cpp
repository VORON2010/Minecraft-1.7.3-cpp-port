#include "client/gui/ContainerScreen.h"

#include "client/Minecraft.h"
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
	jstring key;
	Item *item = stack.getItem();
	if (item != nullptr)
	{
		key = item->getDescriptionId(stack);
	}
	else if (stack.itemID > 0 && stack.itemID < static_cast<int_t>(Tile::tiles.size()) && Tile::tiles[stack.itemID] != nullptr)
	{
		
		key = Tile::tiles[stack.itemID]->descriptionId;
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

void ContainerScreen::tick()
{
	Screen::tick();
	if (minecraft.player != nullptr &&
		shouldClose(minecraft.player->isAlive(), minecraft.player->removed))
	{
		minecraft.player->closeContainer();
	}
}
