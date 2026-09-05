#pragma once

#include <vector>

#include "world/inventory/IInventory.h"
#include "world/item/crafting/CraftingContainer.h"

class Container;

class CraftingInventory : public IInventory, public CraftingContainer
{
private:
	std::vector<ItemInstance> items;
	int_t width;
	Container &eventHandler;

public:
	CraftingInventory(Container &eventHandler, int_t width, int_t height);

	int_t getSizeInventory() const override;
	ItemInstance *getStackInSlot(int_t slot) override;
	const ItemInstance *getStackInSlot(int_t slot) const override;
	ItemInstance getItem(int_t x, int_t y) const override;
	ItemInstance decrStackSize(int_t slot, int_t count) override;
	void setInventorySlotContents(int_t slot, const ItemInstance &item) override;
	jstring getInvName() const override;
	void onInventoryChanged() override;
	bool canInteractWith(Player &player) const override;
};

class CraftResultInventory : public IInventory
{
private:
	ItemInstance result;

public:
	int_t getSizeInventory() const override;
	ItemInstance *getStackInSlot(int_t slot) override;
	const ItemInstance *getStackInSlot(int_t slot) const override;
	ItemInstance decrStackSize(int_t slot, int_t count) override;
	void setInventorySlotContents(int_t slot, const ItemInstance &item) override;
	jstring getInvName() const override;
	void onInventoryChanged() override;
	bool canInteractWith(Player &player) const override;
};
