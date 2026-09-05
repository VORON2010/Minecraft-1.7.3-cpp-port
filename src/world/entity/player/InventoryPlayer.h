#pragma once

#include <array>
#include <memory>

#include "java/Type.h"
#include "world/inventory/IInventory.h"
#include "world/item/ItemInstance.h"

class Player;
class ListTag;
class Tile;
class Entity;

class InventoryPlayer : public IInventory
{
public:
	std::array<ItemInstance, 36> mainInventory = {};
	std::array<ItemInstance, 4> armorInventory = {};
	int_t currentItem = 0;
	bool inventoryChanged = false;


	InventoryPlayer(Player *player = nullptr);

	ItemInstance *getCurrentItem();
	ItemInstance *getSelected();
	ItemInstance *getItem(int_t slot);
	const ItemInstance *getItem(int_t slot) const;
	ItemInstance removeItem(int_t slot, int_t count);
	void setItem(int_t slot, const ItemInstance &item);
	void changeCurrentItem(int_t direction);
	void setCurrentItem(int_t itemId);
	void tick();
	bool add(ItemInstance &item);
	bool consumeItem(int_t itemId);
	void dropAll();
	float getDestroySpeed(Tile &tile);
	bool canDestroySpecial(Tile &tile);
	int_t getAttackDamage(Entity &entity);
	int_t getArmorValue() const;
	void hurtArmor(int_t damage);
	void save(ListTag &tag) const;
	void load(ListTag &tag);

	ItemInstance *getCarried();
	void setCarried(const ItemInstance &item);
	void setCarried(ItemInstance &&item);
	void setCarriedNull();
	int_t getContainerSize() const;
	jstring getName() const;
	void setChanged();
	bool canUse(Player &player) const;
	Player &getPlayer() const;

	int_t getSizeInventory() const override { return getContainerSize(); }
	ItemInstance *getStackInSlot(int_t slot) override { return getItem(slot); }
	const ItemInstance *getStackInSlot(int_t slot) const override { return getItem(slot); }
	ItemInstance decrStackSize(int_t slot, int_t count) override { return removeItem(slot, count); }
	void setInventorySlotContents(int_t slot, const ItemInstance &item) override { setItem(slot, item); }
	jstring getInvName() const override { return getName(); }
	void onInventoryChanged() override { setChanged(); }
	bool canInteractWith(Player &player) const override { return canUse(player); }

private:
	Player *player = nullptr;
	ItemInstance carried = {};
	int_t getFreeSlot() const;
	int_t getSlotWithRemainingSpace(const ItemInstance &item) const;
};
