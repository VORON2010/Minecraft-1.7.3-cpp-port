#include "world/inventory/BasicInventory.h"

#include <utility>

BasicInventory::BasicInventory(jstring title, int_t size)
	: title(std::move(title)), items(static_cast<std::size_t>(size))
{
}

int_t BasicInventory::getSizeInventory() const
{
	return static_cast<int_t>(items.size());
}

ItemInstance *BasicInventory::getStackInSlot(int_t slot)
{
	ItemInstance &item = items.at(static_cast<std::size_t>(slot));
	return item.isEmpty() ? nullptr : &item;
}

const ItemInstance *BasicInventory::getStackInSlot(int_t slot) const
{
	const ItemInstance &item = items.at(static_cast<std::size_t>(slot));
	return item.isEmpty() ? nullptr : &item;
}

ItemInstance BasicInventory::decrStackSize(int_t slot, int_t count)
{
	ItemInstance &item = items.at(static_cast<std::size_t>(slot));
	if (item.isEmpty())
		return ItemInstance();
	if (item.stackSize <= count)
	{
		ItemInstance removed = item;
		item = ItemInstance();
		onInventoryChanged();
		return removed;
	}
	ItemInstance removed = item.remove(count);
	if (item.stackSize == 0)
		item = ItemInstance();
	onInventoryChanged();
	return removed;
}

void BasicInventory::setInventorySlotContents(int_t slot, const ItemInstance &item)
{
	items.at(static_cast<std::size_t>(slot)) = item;
	ItemInstance &stored = items.at(static_cast<std::size_t>(slot));
	if (!stored.isEmpty() && stored.stackSize > getInventoryStackLimit())
		stored.stackSize = getInventoryStackLimit();
	onInventoryChanged();
}

jstring BasicInventory::getInvName() const
{
	return title;
}

void BasicInventory::onInventoryChanged()
{
}

bool BasicInventory::canInteractWith(Player &player) const
{
	(void)player;
	return true;
}
