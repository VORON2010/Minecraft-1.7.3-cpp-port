#pragma once

#include <vector>

#include "world/inventory/IInventory.h"

class BasicInventory : public IInventory
{
private:
	jstring title;
	std::vector<ItemInstance> items;

public:
	BasicInventory(jstring title, int_t size);
	int_t getContainerSize() const { return getSizeInventory(); }
	ItemInstance &getItem(int_t slot) { return items[static_cast<std::size_t>(slot)]; }
	const ItemInstance &getItem(int_t slot) const { return items[static_cast<std::size_t>(slot)]; }
	void setChanged() { onInventoryChanged(); }
	bool canUse(Player &player) const { return canInteractWith(player); }
	const jstring &getName() const { return title; }

	int_t getSizeInventory() const override;
	ItemInstance *getStackInSlot(int_t slot) override;
	const ItemInstance *getStackInSlot(int_t slot) const override;
	ItemInstance decrStackSize(int_t slot, int_t count) override;
	void setInventorySlotContents(int_t slot, const ItemInstance &item) override;
	jstring getInvName() const override;
	void onInventoryChanged() override;
	bool canInteractWith(Player &player) const override;
};
