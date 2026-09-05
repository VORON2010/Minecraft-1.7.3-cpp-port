#pragma once

#include "java/String.h"
#include "java/Type.h"
#include "world/item/ItemInstance.h"

class Player;

class IInventory
{
public:
	virtual ~IInventory() = default;

	virtual int_t getSizeInventory() const = 0;
	virtual ItemInstance *getStackInSlot(int_t slot) = 0;
	virtual const ItemInstance *getStackInSlot(int_t slot) const = 0;
	virtual ItemInstance decrStackSize(int_t slot, int_t count) = 0;
	virtual void setInventorySlotContents(int_t slot, const ItemInstance &item) = 0;
	virtual jstring getInvName() const = 0;
	virtual int_t getInventoryStackLimit() const { return 64; }
	virtual void onInventoryChanged() = 0;
	virtual bool canInteractWith(Player &player) const = 0;
};
