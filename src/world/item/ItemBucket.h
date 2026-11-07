#pragma once

#include "world/item/Item.h"

class ItemBucket : public Item
{
private:
	int_t containedBlockId = 0;

public:
	ItemBucket(int_t baseId, int_t containedBlockId);
	void use(ItemInstance &stack, Level &level, Player &player) const override;
	void useTimeDepleted(ItemInstance &stack, Level &level, Player &player) const override;
	UseAnim getUseAnimation(const ItemInstance &stack) const override { (void)stack; return containedBlockId < 0 ? UseAnim::DRINK : UseAnim::NONE; }
	int_t getUseDuration(const ItemInstance &stack) const override { (void)stack; return containedBlockId < 0 ? 32 : 0; }
};
