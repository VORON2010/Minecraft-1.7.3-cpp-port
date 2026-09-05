#pragma once

#include "java/Type.h"

class ContainerPlayer;
class IInventory;
class ItemInstance;
class Player;

class Slot
{
private:
	int_t slotIndex;
	IInventory &inventory;

public:
	int_t slotNumber = 0;
	int_t x = 0;
	int_t y = 0;

	Slot(IInventory &inventory, int_t slotIndex, int_t x, int_t y);
	virtual ~Slot() = default;

	virtual void onPickup(ItemInstance &item);
	virtual bool mayPlace(const ItemInstance &item) const;
	ItemInstance *getItem();
	const ItemInstance *getItem() const;
	bool hasItem() const;
	void set(const ItemInstance *item);
	void setChanged();
	virtual int_t getMaxStackSize() const;
	virtual int_t getBackgroundIcon() const;
	ItemInstance remove(int_t count);
};

class ArmorSlot : public Slot
{
private:
	int_t armorType;

public:
	ArmorSlot(IInventory &inventory, int_t slotIndex, int_t x, int_t y, int_t armorType);
	int_t getMaxStackSize() const override;
	bool mayPlace(const ItemInstance &item) const override;
};

class CraftingSlot : public Slot
{
private:
	IInventory &craftMatrix;
	Player &player;

public:
	CraftingSlot(Player &player, IInventory &craftMatrix, IInventory &result,
		int_t slotIndex, int_t x, int_t y);
	bool mayPlace(const ItemInstance &item) const override;
	void onPickup(ItemInstance &item) override;
};

class FurnaceResultSlot : public Slot
{
private:
	Player &player;

public:
	FurnaceResultSlot(Player &player, IInventory &inventory, int_t slotIndex, int_t x, int_t y);
	bool mayPlace(const ItemInstance &item) const override;
	void onPickup(ItemInstance &item) override;
};
