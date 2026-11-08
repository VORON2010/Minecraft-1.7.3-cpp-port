#pragma once

#include "world/item/Item.h"

// 1.5.2 WritableBookItem (386) and WrittenBookItem (387): pages and title live in the item tag
class WritableBookItem : public Item
{
public:
	using Item::Item;
};

class WrittenBookItem : public Item
{
public:
	using Item::Item;
	bool isFoil(const ItemInstance &stack) const override { (void)stack; return true; }
	jstring getHoverName(const ItemInstance &stack) const override;
	void appendHoverText(const ItemInstance &stack, std::vector<jstring> &lines) const override;
};
