#pragma once

#include <array>

#include "world/inventory/IInventory.h"
#include "world/item/ItemInstance.h"
#include "world/level/tile/entity/TileEntity.h"

class Player;

// 1.0 BrewingStandBlockEntity: slots 0-2 bottles, slot 3 the ingredient, 600 ticks per brew
class BrewingStandTileEntity : public TileEntity, public IInventory
{
	std::array<ItemInstance, 4> items = {};
	int_t brewTime = 0;
	int_t lastPotionCount = 0;
	int_t ingredientId = 0;

	bool isBrewable() const;
	void doBrew();
	int_t applyIngredient(int_t data, const ItemInstance &ingredient) const;

public:
	jstring getEncodeId() const override { return u"Cauldron"; }
	void load(CompoundTag &tag) override;
	void save(CompoundTag &tag) override;
	void tick() override;

	int_t getBrewTime() const { return brewTime; }
	void setBrewTime(int_t value) { brewTime = value; }
	int_t getPotionBits() const;
	ItemInstance &getItem(int_t slot) { return items[slot]; }

	int_t getSizeInventory() const override { return static_cast<int_t>(items.size()); }
	ItemInstance *getStackInSlot(int_t slot) override { return slot >= 0 && slot < 4 && !items[slot].isEmpty() ? &items[slot] : nullptr; }
	const ItemInstance *getStackInSlot(int_t slot) const override { return slot >= 0 && slot < 4 && !items[slot].isEmpty() ? &items[slot] : nullptr; }
	ItemInstance decrStackSize(int_t slot, int_t count) override;
	void setInventorySlotContents(int_t slot, const ItemInstance &item) override;
	jstring getInvName() const override { return u"Brewing Stand"; }
	int_t getInventoryStackLimit() const override { return 1; }
	void onInventoryChanged() override { TileEntity::setChanged(); }
	bool canInteractWith(Player &player) const override;
};
