#pragma once

#include <memory>

#include "world/inventory/Container.h"
#include "world/inventory/CraftingInventory.h"

class FurnaceTileEntity;
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
