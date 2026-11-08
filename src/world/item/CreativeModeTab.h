#pragma once

#include <vector>

#include "java/String.h"
#include "java/Type.h"
#include "world/item/ItemInstance.h"

// 1.5.2 CreativeModeTab: the twelve tabs of the creative inventory
namespace CreativeModeTab
{
	enum
	{
		BUILDING_BLOCKS = 0,
		DECORATIONS,
		REDSTONE,
		TRANSPORTATION,
		MISC,
		SEARCH,
		FOOD,
		TOOLS,
		COMBAT,
		BREWING,
		MATERIALS,
		INVENTORY,
		COUNT
	};

	// -1 when the item is not in the creative inventory
	int_t getTab(int_t id);
	jstring getName(int_t tab);
	int_t getIconItem(int_t tab);
	bool hasScrollbar(int_t tab);
	bool hasTooltips(int_t tab);
	const char16_t *getTexture(int_t tab);
	inline int_t getColumn(int_t tab) { return tab % 6; }
	inline bool isTopRow(int_t tab) { return tab < 6; }

	// Item.addToCreativeMenu for one id: one stack per variant
	void addSubtypes(int_t id, std::vector<ItemInstance> &out);
	// CreativeModeTab.addItems
	void addItems(int_t tab, std::vector<ItemInstance> &out);
	// everything the search tab looks through
	void addAllItems(std::vector<ItemInstance> &out);
}
