#pragma once

#include <array>

#include "world/level/tile/entity/TileEntity.h"
#include "world/inventory/IInventory.h"
#include "world/item/ItemInstance.h"

class Player;

class FurnaceTileEntity : public TileEntity, public IInventory
{
private:
	std::array<ItemInstance, 3> items = {};

	bool canSmelt() const;
	void smeltItem();
	int_t getBurnDuration(const ItemInstance &stack) const;

public:
	int_t burnTime = 0;
	int_t currentItemBurnTime = 0;
	int_t cookTime = 0;

	jstring getEncodeId() const override { return u"Furnace"; }
	void load(CompoundTag &tag) override;
	void save(CompoundTag &tag) override;
	void tick() override;

	ItemInstance &getItem(int_t slot) { return items[slot]; }
	const ItemInstance &getItem(int_t slot) const { return items[slot]; }
	int_t getCookProgressScaled(int_t scale) const;
	int_t getBurnTimeRemainingScaled(int_t scale) const;
	bool isBurning() const;
	bool canUse(Player &player) const;
	void setItem(int_t slot, const ItemInstance &item);
	ItemInstance removeItem(int_t slot, int_t count);

	int_t getSizeInventory() const override { return static_cast<int_t>(items.size()); }
	ItemInstance *getStackInSlot(int_t slot) override { return getItem(slot).isEmpty() ? nullptr : &getItem(slot); }
	const ItemInstance *getStackInSlot(int_t slot) const override { return getItem(slot).isEmpty() ? nullptr : &getItem(slot); }
	ItemInstance decrStackSize(int_t slot, int_t count) override { return removeItem(slot, count); }
	void setInventorySlotContents(int_t slot, const ItemInstance &item) override { setItem(slot, item); }
	jstring getInvName() const override { return u"Furnace"; }
	void onInventoryChanged() override { TileEntity::setChanged(); }
	bool canInteractWith(Player &player) const override { return canUse(player); }
};
