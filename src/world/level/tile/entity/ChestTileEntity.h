#pragma once

#include <array>

#include "world/level/tile/entity/TileEntity.h"
#include "world/inventory/IInventory.h"
#include "world/item/ItemInstance.h"

class Player;

class ChestTileEntity : public TileEntity, public IInventory
{
private:
	std::array<ItemInstance, 27> items = {};

public:
	jstring getEncodeId() const override { return u"Chest"; }
	void load(CompoundTag &tag) override;
	void save(CompoundTag &tag) override;
	ItemInstance &getItem(int_t slot);
	const ItemInstance &getItem(int_t slot) const;
	void setItem(int_t slot, const ItemInstance &item);
	ItemInstance removeItem(int_t slot, int_t count);
	int_t getContainerSize() const;
	bool canUse(Player &player) const;
	jstring getName() const;

	int_t getSizeInventory() const override { return getContainerSize(); }
	ItemInstance *getStackInSlot(int_t slot) override { return getItem(slot).isEmpty() ? nullptr : &getItem(slot); }
	const ItemInstance *getStackInSlot(int_t slot) const override { return getItem(slot).isEmpty() ? nullptr : &getItem(slot); }
	ItemInstance decrStackSize(int_t slot, int_t count) override { return removeItem(slot, count); }
	void setInventorySlotContents(int_t slot, const ItemInstance &item) override { setItem(slot, item); }
	jstring getInvName() const override { return getName(); }
	void onInventoryChanged() override { TileEntity::setChanged(); }
	bool canInteractWith(Player &player) const override { return canUse(player); }
};
