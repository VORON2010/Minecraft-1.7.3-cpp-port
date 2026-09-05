#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "java/Type.h"
#include "world/item/ItemInstance.h"
#include "world/inventory/Slot.h"

class IInventory;
class InventoryPlayer;
class Player;

class Container
{
private:
	std::uint16_t actionNumber = 0;

protected:
	void addSlot(std::unique_ptr<Slot> slot);
	bool moveItemStackTo(ItemInstance &item, int_t firstSlot, int_t lastSlot, bool reverse);
	std::unique_ptr<ItemInstance> quickMoveStackTo(int_t slot, int_t firstSlot,
		int_t lastSlot, bool reverse);

public:
	std::vector<ItemInstance> lastSlots;
	std::vector<std::unique_ptr<Slot>> slots;
	int_t windowId = 0;

	virtual ~Container() = default;

	void updateCraftingResults();
	Slot &getSlot(int_t slot);
	const Slot &getSlot(int_t slot) const;
	std::unique_ptr<ItemInstance> click(int_t slot, int_t button, bool shiftClick, Player &player);
	virtual void onClosed(Player &player);
	virtual void onCraftMatrixChanged(IInventory &inventory);
	void putStackInSlot(int_t slot, const ItemInstance *item);
	void putStacksInSlots(const std::vector<std::unique_ptr<ItemInstance>> &items);
	virtual void updateProgressBar(int_t id, int_t value);
	short_t getNextTransactionId(InventoryPlayer &inventory);
	virtual void transactionAccepted(short_t action);
	virtual void transactionRejected(short_t action);
	virtual bool canUse(Player &player) const = 0;
	virtual std::unique_ptr<ItemInstance> quickMoveStack(int_t slot);
};
