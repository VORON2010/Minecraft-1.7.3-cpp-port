#pragma once

#include <memory>

#include "java/String.h"
#include "world/item/ItemInstance.h"
#include "world/inventory/IInventory.h"

class Player;
class ChestTileEntity;

class CompoundContainer : public IInventory
{
private:
	jstring name;
	std::shared_ptr<ChestTileEntity> first;
	std::shared_ptr<ChestTileEntity> second;

public:
	CompoundContainer(jstring name, std::shared_ptr<ChestTileEntity> first, std::shared_ptr<ChestTileEntity> second);
	int_t getContainerSize() const;
	ItemInstance &getItem(int_t slot);
	const ItemInstance &getItem(int_t slot) const;
	void setItem(int_t slot, const ItemInstance &item);
	ItemInstance removeItem(int_t slot, int_t count);
	bool canUse(Player &player) const;
	void setChanged();
	const jstring &getName() const;

	int_t getSizeInventory() const override { return getContainerSize(); }
	ItemInstance *getStackInSlot(int_t slot) override { return getItem(slot).isEmpty() ? nullptr : &getItem(slot); }
	const ItemInstance *getStackInSlot(int_t slot) const override { return getItem(slot).isEmpty() ? nullptr : &getItem(slot); }
	ItemInstance decrStackSize(int_t slot, int_t count) override { return removeItem(slot, count); }
	void setInventorySlotContents(int_t slot, const ItemInstance &item) override { setItem(slot, item); }
	jstring getInvName() const override { return getName(); }
	void onInventoryChanged() override { setChanged(); }
	bool canInteractWith(Player &player) const override { return canUse(player); }
};
