#include "world/inventory/Container.h"

#include <cstring>

#include "world/entity/player/InventoryPlayer.h"
#include "world/entity/player/Player.h"
#include "world/inventory/IInventory.h"
#include "world/inventory/Slot.h"

namespace
{
	bool stacksEqual(const ItemInstance &left, const ItemInstance &right)
	{
		return left.stackSize == right.stackSize
			&& left.itemID == right.itemID
			&& left.itemDamage == right.itemDamage;
	}
}

void Container::addSlot(std::unique_ptr<Slot> slot)
{
	slot->slotNumber = static_cast<int_t>(slots.size());
	slots.push_back(std::move(slot));
	lastSlots.emplace_back();
}

void Container::updateCraftingResults()
{
	for (std::size_t index = 0; index < slots.size(); ++index)
	{
		const ItemInstance *item = slots[index]->getItem();
		ItemInstance current = item == nullptr ? ItemInstance() : *item;
		if (!stacksEqual(lastSlots[index], current))
			lastSlots[index] = current;
	}
}

Slot &Container::getSlot(int_t slot)
{
	return *slots.at(static_cast<std::size_t>(slot));
}

const Slot &Container::getSlot(int_t slot) const
{
	return *slots.at(static_cast<std::size_t>(slot));
}

std::unique_ptr<ItemInstance> Container::click(int_t slotNumber, int_t button, bool shiftClick, Player &player)
{
	std::unique_ptr<ItemInstance> result;
	if (button != 0 && button != 1)
		return result;

	InventoryPlayer &inventory = player.inventory;
	if (slotNumber == -999)
	{
		ItemInstance *carried = inventory.getCarried();
		if (carried != nullptr)
		{
			if (button == 0)
			{
				player.drop(*carried);
				inventory.setCarriedNull();
			}
			if (button == 1)
			{
				ItemInstance dropped = carried->remove(1);
				player.drop(dropped);
				if (carried->stackSize == 0)
					inventory.setCarriedNull();
			}
		}
		return result;
	}

	if (shiftClick)
	{
		result = quickMoveStack(slotNumber);
		if (result != nullptr)
		{
			int_t originalCount = result->stackSize;
			Slot &slot = getSlot(slotNumber);
			if (slot.getItem() != nullptr && slot.getItem()->stackSize < originalCount)
				click(slotNumber, button, true, player);
		}
		return result;
	}

	Slot &slot = getSlot(slotNumber);
	slot.setChanged();
	ItemInstance *slotItem = slot.getItem();
	ItemInstance *carried = inventory.getCarried();
	if (slotItem != nullptr)
		result = std::make_unique<ItemInstance>(*slotItem);

	if (slotItem == nullptr)
	{
		if (carried != nullptr && slot.mayPlace(*carried))
		{
			int_t count = button == 0 ? carried->stackSize.load(std::memory_order_relaxed) : 1;
			if (count > slot.getMaxStackSize())
				count = slot.getMaxStackSize();
			ItemInstance placed = carried->remove(count);
			slot.set(&placed);
			if (carried->stackSize == 0)
				inventory.setCarriedNull();
		}
	}
	else if (carried == nullptr)
	{
		int_t count = button == 0 ? slotItem->stackSize.load(std::memory_order_relaxed)
			: (slotItem->stackSize + 1) / 2;
		ItemInstance removed = slot.remove(count);
		inventory.setCarried(std::move(removed));
		if (slotItem->stackSize == 0)
			slot.set(nullptr);
		ItemInstance *newCarried = inventory.getCarried();
		if (newCarried != nullptr)
			slot.onPickup(*newCarried);
	}
	else if (slot.mayPlace(*carried))
	{
		if (slotItem->itemID != carried->itemID
			|| (slotItem->getHasSubtypes() && slotItem->itemDamage != carried->itemDamage))
		{
			if (carried->stackSize <= slot.getMaxStackSize())
			{
				ItemInstance oldSlot = *slotItem;
				slot.set(carried);
				inventory.setCarried(std::move(oldSlot));
			}
		}
		else
		{
			int_t count = button == 0 ? carried->stackSize.load(std::memory_order_relaxed) : 1;
			if (count > slot.getMaxStackSize() - slotItem->stackSize)
				count = slot.getMaxStackSize() - slotItem->stackSize;
			if (count > carried->getMaxStackSize() - slotItem->stackSize)
				count = carried->getMaxStackSize() - slotItem->stackSize;
			carried->remove(count);
			if (carried->stackSize == 0)
				inventory.setCarriedNull();
			slotItem->stackSize += count;
		}
	}
	else if (slotItem->itemID == carried->itemID && carried->getMaxStackSize() > 1
		&& (!slotItem->getHasSubtypes() || slotItem->itemDamage == carried->itemDamage))
	{
		int_t count = slotItem->stackSize;
		if (count > 0 && count + carried->stackSize <= carried->getMaxStackSize())
		{
			carried->stackSize += count;
			slotItem->remove(count);
			if (slotItem->stackSize == 0)
				slot.set(nullptr);
			ItemInstance *newCarried = inventory.getCarried();
			if (newCarried != nullptr)
				slot.onPickup(*newCarried);
		}
	}

	return result;
}

void Container::onClosed(Player &player)
{
	ItemInstance *carried = player.inventory.getCarried();
	if (carried != nullptr)
	{
		player.drop(*carried);
		player.inventory.setCarriedNull();
	}
}

void Container::onCraftMatrixChanged(IInventory &inventory)
{
	(void)inventory;
	updateCraftingResults();
}

void Container::putStackInSlot(int_t slot, const ItemInstance *item)
{
	getSlot(slot).set(item);
}

void Container::putStacksInSlots(const std::vector<std::unique_ptr<ItemInstance>> &items)
{
	for (std::size_t slot = 0; slot < items.size(); ++slot)
		getSlot(static_cast<int_t>(slot)).set(items[slot].get());
}

void Container::updateProgressBar(int_t id, int_t value)
{
	(void)id;
	(void)value;
}

short_t Container::getNextTransactionId(InventoryPlayer &inventory)
{
	(void)inventory;
	++actionNumber;
	short_t result;
	std::memcpy(&result, &actionNumber, sizeof(result));
	return result;
}

void Container::transactionAccepted(short_t action)
{
	(void)action;
}

void Container::transactionRejected(short_t action)
{
	(void)action;
}

std::unique_ptr<ItemInstance> Container::quickMoveStack(int_t slot)
{
	ItemInstance *item = getSlot(slot).getItem();
	return item == nullptr ? nullptr : std::make_unique<ItemInstance>(*item);
}

std::unique_ptr<ItemInstance> Container::quickMoveStackTo(int_t slotNumber,
	int_t firstSlot, int_t lastSlot, bool reverse)
{
	Slot &slot = getSlot(slotNumber);
	ItemInstance *slotItem = slot.getItem();
	if (slotItem == nullptr)
		return nullptr;
	ItemInstance original = *slotItem;
	moveItemStackTo(*slotItem, firstSlot, lastSlot, reverse);
	ItemInstance remaining = *slotItem;
	if (slotItem->stackSize == 0)
		slot.set(nullptr);
	else
		slot.setChanged();
	if (remaining.stackSize == original.stackSize)
		return nullptr;
	slot.onPickup(remaining);
	return std::make_unique<ItemInstance>(original);
}

bool Container::moveItemStackTo(ItemInstance &item, int_t firstSlot, int_t lastSlot, bool reverse)
{
	int_t slotIndex = reverse ? lastSlot - 1 : firstSlot;
	if (item.isStackable())
	{
		while (item.stackSize > 0 && ((!reverse && slotIndex < lastSlot) || (reverse && slotIndex >= firstSlot)))
		{
			Slot &slot = getSlot(slotIndex);
			ItemInstance *slotItem = slot.getItem();
			if (slotItem != nullptr && slotItem->itemID == item.itemID
				&& (!item.getHasSubtypes() || slotItem->itemDamage == item.itemDamage))
			{
				int_t total = slotItem->stackSize + item.stackSize;
				if (total <= item.getMaxStackSize())
				{
					item.stackSize = 0;
					slotItem->stackSize = total;
					slot.setChanged();
				}
				else if (slotItem->stackSize < item.getMaxStackSize())
				{
					item.stackSize -= item.getMaxStackSize() - slotItem->stackSize;
					slotItem->stackSize = item.getMaxStackSize();
					slot.setChanged();
				}
			}
			slotIndex += reverse ? -1 : 1;
		}
	}

	if (item.stackSize > 0)
	{
		slotIndex = reverse ? lastSlot - 1 : firstSlot;
		while ((!reverse && slotIndex < lastSlot) || (reverse && slotIndex >= firstSlot))
		{
			Slot &slot = getSlot(slotIndex);
			if (slot.getItem() == nullptr)
			{
				ItemInstance copy = item;
				slot.set(&copy);
				slot.setChanged();
				item.stackSize = 0;
				break;
			}
			slotIndex += reverse ? -1 : 1;
		}
	}
	return item.stackSize == 0;
}
