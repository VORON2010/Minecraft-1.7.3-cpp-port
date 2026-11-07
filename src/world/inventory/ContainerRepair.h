#pragma once

#include <memory>

#include "java/String.h"
#include "world/inventory/Container.h"
#include "world/inventory/IInventory.h"

class InventoryPlayer;
class Level;
class Player;
class ContainerRepair;

// two input slots that tell the anvil menu when they change
class RepairInputInventory : public IInventory
{
	ItemInstance items[2];
	ContainerRepair &menu;

public:
	explicit RepairInputInventory(ContainerRepair &menu) : menu(menu) {}
	int_t getSizeInventory() const override { return 2; }
	ItemInstance *getStackInSlot(int_t slot) override { return slot >= 0 && slot < 2 && !items[slot].isEmpty() ? &items[slot] : nullptr; }
	const ItemInstance *getStackInSlot(int_t slot) const override { return slot >= 0 && slot < 2 && !items[slot].isEmpty() ? &items[slot] : nullptr; }
	ItemInstance decrStackSize(int_t slot, int_t count) override;
	void setInventorySlotContents(int_t slot, const ItemInstance &stack) override;
	jstring getInvName() const override { return u"Repair"; }
	void onInventoryChanged() override;
	bool canInteractWith(Player &player) const override { (void)player; return true; }
};

class RepairResultInventory : public IInventory
{
	ItemInstance item;

public:
	int_t getSizeInventory() const override { return 1; }
	ItemInstance *getStackInSlot(int_t slot) override { return slot == 0 && !item.isEmpty() ? &item : nullptr; }
	const ItemInstance *getStackInSlot(int_t slot) const override { return slot == 0 && !item.isEmpty() ? &item : nullptr; }
	ItemInstance decrStackSize(int_t slot, int_t count) override;
	void setInventorySlotContents(int_t slot, const ItemInstance &stack) override { if (slot == 0) item = stack; }
	jstring getInvName() const override { return u"Result"; }
	void onInventoryChanged() override {}
	bool canInteractWith(Player &player) const override { (void)player; return true; }
};

// 1.4.7 AnvilMenu: repair, combine enchantments, rename
class ContainerRepair : public Container
{
	Level &level;
	int_t x, y, z;
	Player &player;
	jstring itemName;
	bool hasName = false;

public:
	RepairInputInventory inputs;
	RepairResultInventory result;
	int_t cost = 0;
	int_t repairItemCountCost = 0;
	Player *lastPlayer = nullptr;

	ContainerRepair(InventoryPlayer &inventory, Level &level, int_t x, int_t y, int_t z, Player &player);
	void createResult();
	void setItemName(const jstring &name);
	void onTake(Player &taker);
	void onClosed(Player &taker) override;
	bool canUse(Player &taker) const override;
	std::unique_ptr<ItemInstance> quickMoveStack(int_t slot) override;
	void updateProgressBar(int_t id, int_t value) override { if (id == 0) cost = value; }
};
