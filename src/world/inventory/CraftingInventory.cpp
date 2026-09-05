#include "world/inventory/CraftingInventory.h"

#include <stdexcept>

#include "world/inventory/Container.h"

CraftingInventory::CraftingInventory(Container &eventHandler, int_t width, int_t height)
	: items(static_cast<std::size_t>(width * height)), width(width), eventHandler(eventHandler)
{
}

int_t CraftingInventory::getSizeInventory() const
{
	return static_cast<int_t>(items.size());
}

ItemInstance *CraftingInventory::getStackInSlot(int_t slot)
{
	if (slot >= getSizeInventory())
		return nullptr;
	ItemInstance &item = items.at(static_cast<std::size_t>(slot));
	return item.isEmpty() ? nullptr : &item;
}

const ItemInstance *CraftingInventory::getStackInSlot(int_t slot) const
{
	if (slot >= getSizeInventory())
		return nullptr;
	const ItemInstance &item = items.at(static_cast<std::size_t>(slot));
	return item.isEmpty() ? nullptr : &item;
}

ItemInstance CraftingInventory::getItem(int_t x, int_t y) const
{
	if (x < 0 || x >= width)
		return ItemInstance();
	int_t slot = x + y * width;
	const ItemInstance *item = getStackInSlot(slot);
	return item == nullptr ? ItemInstance() : *item;
}

ItemInstance CraftingInventory::decrStackSize(int_t slot, int_t count)
{
	ItemInstance &item = items.at(static_cast<std::size_t>(slot));
	if (item.isEmpty())
		return ItemInstance();
	if (item.stackSize <= count)
	{
		ItemInstance removed = item;
		item = ItemInstance();
		eventHandler.onCraftMatrixChanged(*this);
		return removed;
	}
	ItemInstance removed = item.remove(count);
	if (item.stackSize == 0)
		item = ItemInstance();
	eventHandler.onCraftMatrixChanged(*this);
	return removed;
}

void CraftingInventory::setInventorySlotContents(int_t slot, const ItemInstance &item)
{
	items.at(static_cast<std::size_t>(slot)) = item;
	eventHandler.onCraftMatrixChanged(*this);
}

jstring CraftingInventory::getInvName() const
{
	return u"Crafting";
}

void CraftingInventory::onInventoryChanged()
{
}

bool CraftingInventory::canInteractWith(Player &player) const
{
	(void)player;
	return true;
}

int_t CraftResultInventory::getSizeInventory() const
{
	return 1;
}

ItemInstance *CraftResultInventory::getStackInSlot(int_t slot)
{
	if (slot != 0)
		throw std::out_of_range("Inventory slot out of range");
	return result.isEmpty() ? nullptr : &result;
}

const ItemInstance *CraftResultInventory::getStackInSlot(int_t slot) const
{
	if (slot != 0)
		throw std::out_of_range("Inventory slot out of range");
	return result.isEmpty() ? nullptr : &result;
}

ItemInstance CraftResultInventory::decrStackSize(int_t slot, int_t count)
{
	if (slot != 0)
		throw std::out_of_range("Inventory slot out of range");
	(void)count;
	ItemInstance removed = result;
	result = ItemInstance();
	return removed;
}

void CraftResultInventory::setInventorySlotContents(int_t slot, const ItemInstance &item)
{
	if (slot != 0)
		throw std::out_of_range("Inventory slot out of range");
	result = item;
}

jstring CraftResultInventory::getInvName() const
{
	return u"Result";
}

void CraftResultInventory::onInventoryChanged()
{
}

bool CraftResultInventory::canInteractWith(Player &player) const
{
	(void)player;
	return true;
}
