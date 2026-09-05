#include "world/inventory/ContainerMenus.h"

#include "world/entity/player/InventoryPlayer.h"
#include "world/entity/player/Player.h"
#include "world/inventory/IInventory.h"
#include "world/inventory/Slot.h"
#include "world/item/crafting/Recipes.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/WorkbenchTile.h"
#include "world/level/tile/entity/FurnaceTileEntity.h"

ContainerPlayer::ContainerPlayer(InventoryPlayer &inventory)
	: ContainerPlayer(inventory, true)
{
}

ContainerPlayer::ContainerPlayer(InventoryPlayer &inventory, bool singlePlayer)
	: craftMatrix(*this, 2, 2), isSinglePlayer(singlePlayer)
{
	Player &player = inventory.getPlayer();
	addSlot(std::make_unique<CraftingSlot>(player, craftMatrix, craftResult, 0, 144, 36));
	for (int_t row = 0; row < 2; ++row)
		for (int_t column = 0; column < 2; ++column)
			addSlot(std::make_unique<Slot>(craftMatrix, column + row * 2,
				88 + column * 18, 26 + row * 18));
	for (int_t armor = 0; armor < 4; ++armor)
		addSlot(std::make_unique<ArmorSlot>(inventory, inventory.getContainerSize() - 1 - armor,
			8, 8 + armor * 18, armor));
	for (int_t row = 0; row < 3; ++row)
		for (int_t column = 0; column < 9; ++column)
			addSlot(std::make_unique<Slot>(inventory, column + (row + 1) * 9,
				8 + column * 18, 84 + row * 18));
	for (int_t column = 0; column < 9; ++column)
		addSlot(std::make_unique<Slot>(inventory, column, 8 + column * 18, 142));
	onCraftMatrixChanged(craftMatrix);
}

void ContainerPlayer::onCraftMatrixChanged(IInventory &inventory)
{
	(void)inventory;
	craftResult.setInventorySlotContents(0, Recipes::getInstance().getItemFor(craftMatrix));
}

void ContainerPlayer::onClosed(Player &player)
{
	Container::onClosed(player);
	for (int_t slot = 0; slot < 4; ++slot)
	{
		ItemInstance *item = craftMatrix.getStackInSlot(slot);
		if (item != nullptr)
		{
			ItemInstance dropped = *item;
			player.drop(dropped);
			craftMatrix.setInventorySlotContents(slot, ItemInstance());
		}
	}
}

bool ContainerPlayer::canUse(Player &player) const
{
	(void)player;
	return true;
}

std::unique_ptr<ItemInstance> ContainerPlayer::quickMoveStack(int_t slot)
{
	if (slot == 0)
		return quickMoveStackTo(slot, 9, 45, true);
	if (slot >= 9 && slot < 36)
		return quickMoveStackTo(slot, 36, 45, false);
	if (slot >= 36 && slot < 45)
		return quickMoveStackTo(slot, 9, 36, false);
	return quickMoveStackTo(slot, 9, 45, false);
}

ContainerChest::ContainerChest(InventoryPlayer &inventory, std::shared_ptr<IInventory> chest)
	: chest(std::move(chest)), rows(this->chest->getSizeInventory() / 9)
{
	int_t offset = (rows - 4) * 18;
	for (int_t row = 0; row < rows; ++row)
		for (int_t column = 0; column < 9; ++column)
			addSlot(std::make_unique<Slot>(*this->chest, column + row * 9,
				8 + column * 18, 18 + row * 18));
	for (int_t row = 0; row < 3; ++row)
		for (int_t column = 0; column < 9; ++column)
			addSlot(std::make_unique<Slot>(inventory, column + row * 9 + 9,
				8 + column * 18, 103 + row * 18 + offset));
	for (int_t column = 0; column < 9; ++column)
		addSlot(std::make_unique<Slot>(inventory, column, 8 + column * 18, 161 + offset));
}

bool ContainerChest::canUse(Player &player) const
{
	return chest->canInteractWith(player);
}

std::unique_ptr<ItemInstance> ContainerChest::quickMoveStack(int_t slotNumber)
{
	Slot &slot = getSlot(slotNumber);
	ItemInstance *slotItem = slot.getItem();
	if (slotItem == nullptr)
		return nullptr;
	ItemInstance original = *slotItem;
	if (slotNumber < rows * 9)
		moveItemStackTo(*slotItem, rows * 9, static_cast<int_t>(slots.size()), true);
	else
		moveItemStackTo(*slotItem, 0, rows * 9, false);
	if (slotItem->stackSize == 0)
		slot.set(nullptr);
	else
		slot.setChanged();
	return std::make_unique<ItemInstance>(original);
}

ContainerFurnace::ContainerFurnace(InventoryPlayer &inventory, std::shared_ptr<FurnaceTileEntity> furnace)
	: furnace(std::move(furnace))
{
	Player &player = inventory.getPlayer();
	addSlot(std::make_unique<Slot>(*this->furnace, 0, 56, 17));
	addSlot(std::make_unique<Slot>(*this->furnace, 1, 56, 53));
	addSlot(std::make_unique<FurnaceResultSlot>(player, *this->furnace, 2, 116, 35));
	for (int_t row = 0; row < 3; ++row)
		for (int_t column = 0; column < 9; ++column)
			addSlot(std::make_unique<Slot>(inventory, column + row * 9 + 9,
				8 + column * 18, 84 + row * 18));
	for (int_t column = 0; column < 9; ++column)
		addSlot(std::make_unique<Slot>(inventory, column, 8 + column * 18, 142));
}

void ContainerFurnace::updateProgressBar(int_t id, int_t value)
{
	if (id == 0)
		furnace->cookTime = value;
	if (id == 1)
		furnace->burnTime = value;
	if (id == 2)
		furnace->currentItemBurnTime = value;
}

bool ContainerFurnace::canUse(Player &player) const
{
	return furnace->canUse(player);
}

std::unique_ptr<ItemInstance> ContainerFurnace::quickMoveStack(int_t slot)
{
	if (slot == 2)
		return quickMoveStackTo(slot, 3, 39, true);
	if (slot >= 3 && slot < 30)
		return quickMoveStackTo(slot, 30, 39, false);
	if (slot >= 30 && slot < 39)
		return quickMoveStackTo(slot, 3, 30, false);
	return quickMoveStackTo(slot, 3, 39, false);
}

ContainerDispenser::ContainerDispenser(InventoryPlayer &inventory, std::shared_ptr<IInventory> dispenser)
	: dispenser(std::move(dispenser))
{
	for (int_t row = 0; row < 3; ++row)
		for (int_t column = 0; column < 3; ++column)
			addSlot(std::make_unique<Slot>(*this->dispenser, column + row * 3,
				62 + column * 18, 17 + row * 18));
	for (int_t row = 0; row < 3; ++row)
		for (int_t column = 0; column < 9; ++column)
			addSlot(std::make_unique<Slot>(inventory, column + row * 9 + 9,
				8 + column * 18, 84 + row * 18));
	for (int_t column = 0; column < 9; ++column)
		addSlot(std::make_unique<Slot>(inventory, column, 8 + column * 18, 142));
}

bool ContainerDispenser::canUse(Player &player) const
{
	return dispenser->canInteractWith(player);
}

ContainerWorkbench::ContainerWorkbench(InventoryPlayer &inventory, Level &level,
	int_t x, int_t y, int_t z)
	: level(level), x(x), y(y), z(z), craftMatrix(*this, 3, 3)
{
	Player &player = inventory.getPlayer();
	addSlot(std::make_unique<CraftingSlot>(player, craftMatrix, craftResult, 0, 124, 35));
	for (int_t row = 0; row < 3; ++row)
		for (int_t column = 0; column < 3; ++column)
			addSlot(std::make_unique<Slot>(craftMatrix, column + row * 3,
				30 + column * 18, 17 + row * 18));
	for (int_t row = 0; row < 3; ++row)
		for (int_t column = 0; column < 9; ++column)
			addSlot(std::make_unique<Slot>(inventory, column + row * 9 + 9,
				8 + column * 18, 84 + row * 18));
	for (int_t column = 0; column < 9; ++column)
		addSlot(std::make_unique<Slot>(inventory, column, 8 + column * 18, 142));
	onCraftMatrixChanged(craftMatrix);
}

void ContainerWorkbench::onCraftMatrixChanged(IInventory &inventory)
{
	(void)inventory;
	craftResult.setInventorySlotContents(0, Recipes::getInstance().getItemFor(craftMatrix));
}

void ContainerWorkbench::onClosed(Player &player)
{
	Container::onClosed(player);
	if (!level.isOnline)
	{
		for (int_t slot = 0; slot < 9; ++slot)
		{
			ItemInstance *item = craftMatrix.getStackInSlot(slot);
			if (item != nullptr)
			{
				ItemInstance dropped = *item;
				player.drop(dropped);
			}
		}
	}
}

bool ContainerWorkbench::canUse(Player &player) const
{
	return level.getTile(x, y, z) == Tile::workBench.id
		&& player.distanceToSqr(static_cast<double>(x) + 0.5,
			static_cast<double>(y) + 0.5, static_cast<double>(z) + 0.5) <= 64.0;
}

std::unique_ptr<ItemInstance> ContainerWorkbench::quickMoveStack(int_t slot)
{
	if (slot == 0)
		return quickMoveStackTo(slot, 10, 46, true);
	if (slot >= 10 && slot < 37)
		return quickMoveStackTo(slot, 37, 46, false);
	if (slot >= 37 && slot < 46)
		return quickMoveStackTo(slot, 10, 37, false);
	return quickMoveStackTo(slot, 10, 46, false);
}
