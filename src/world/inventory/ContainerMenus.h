#pragma once

#include <memory>

#include "world/inventory/Container.h"
#include "world/inventory/CraftingInventory.h"
#include "java/Random.h"

class FurnaceTileEntity;
class BrewingStandTileEntity;
class IInventory;
class InventoryPlayer;
class Level;

class ContainerPlayer : public Container
{
public:
	CraftingInventory craftMatrix;
	CraftResultInventory craftResult;
	bool isSinglePlayer;

	explicit ContainerPlayer(InventoryPlayer &inventory);
	ContainerPlayer(InventoryPlayer &inventory, bool singlePlayer);

	void onCraftMatrixChanged(IInventory &inventory) override;
	void onClosed(Player &player) override;
	bool canUse(Player &player) const override;
	std::unique_ptr<ItemInstance> quickMoveStack(int_t slot) override;
};

class ContainerChest : public Container
{
private:
	std::shared_ptr<IInventory> chest;
	int_t rows;

public:
	ContainerChest(InventoryPlayer &inventory, std::shared_ptr<IInventory> chest);
	bool canUse(Player &player) const override;
	void onClosed(Player &player) override;
	std::unique_ptr<ItemInstance> quickMoveStack(int_t slot) override;
};

class ContainerFurnace : public Container
{
private:
	std::shared_ptr<FurnaceTileEntity> furnace;

public:
	ContainerFurnace(InventoryPlayer &inventory, std::shared_ptr<FurnaceTileEntity> furnace);
	void updateProgressBar(int_t id, int_t value) override;
	bool canUse(Player &player) const override;
	std::unique_ptr<ItemInstance> quickMoveStack(int_t slot) override;
};

class ContainerDispenser : public Container
{
private:
	std::shared_ptr<IInventory> dispenser;

public:
	ContainerDispenser(InventoryPlayer &inventory, std::shared_ptr<IInventory> dispenser);
	bool canUse(Player &player) const override;
};

class ContainerWorkbench : public Container
{
private:
	Level &level;
	int_t x;
	int_t y;
	int_t z;

public:
	CraftingInventory craftMatrix;
	CraftResultInventory craftResult;

	ContainerWorkbench(InventoryPlayer &inventory, Level &level, int_t x, int_t y, int_t z);
	void onCraftMatrixChanged(IInventory &inventory) override;
	void onClosed(Player &player) override;
	bool canUse(Player &player) const override;
	std::unique_ptr<ItemInstance> quickMoveStack(int_t slot) override;
};

// 1.0 BrewingStandMenu: three bottle slots, one ingredient slot, then the player inventory
class ContainerBrewingStand : public Container
{
private:
	std::shared_ptr<BrewingStandTileEntity> stand;

public:
	ContainerBrewingStand(InventoryPlayer &inventory, std::shared_ptr<BrewingStandTileEntity> stand);
	void updateProgressBar(int_t id, int_t value) override;
	bool canUse(Player &player) const override;
	std::unique_ptr<ItemInstance> quickMoveStack(int_t slot) override;
};

class ContainerEnchantment;

// 1.0 EnchantingTableMenu's one-slot inventory: tells the menu when the item changes
class EnchantInventory : public IInventory
{
	ItemInstance item;
	ContainerEnchantment &menu;

public:
	explicit EnchantInventory(ContainerEnchantment &menu) : menu(menu) {}
	int_t getSizeInventory() const override { return 1; }
	ItemInstance *getStackInSlot(int_t slot) override { return slot == 0 && !item.isEmpty() ? &item : nullptr; }
	const ItemInstance *getStackInSlot(int_t slot) const override { return slot == 0 && !item.isEmpty() ? &item : nullptr; }
	ItemInstance decrStackSize(int_t slot, int_t count) override;
	void setInventorySlotContents(int_t slot, const ItemInstance &stack) override;
	jstring getInvName() const override { return u"Enchant"; }
	int_t getInventoryStackLimit() const override { return 1; }
	void onInventoryChanged() override;
	bool canInteractWith(Player &player) const override { (void)player; return true; }
};

// 1.0 EnchantingTableMenu
class ContainerEnchantment : public Container
{
	Level &level;
	int_t x, y, z;
	Random random;

public:
	EnchantInventory enchantSlots;
	long_t nameSeed = 0;
	int_t costs[3] = {0, 0, 0};

	ContainerEnchantment(InventoryPlayer &inventory, Level &level, int_t x, int_t y, int_t z);
	void slotsChanged();
	bool clickMenuButton(Player &player, int_t id);
	void onClosed(Player &player) override;
	bool canUse(Player &player) const override;
	std::unique_ptr<ItemInstance> quickMoveStack(int_t slot) override;
	void updateProgressBar(int_t id, int_t value) override;
	int_t countBookshelves() const;
};
