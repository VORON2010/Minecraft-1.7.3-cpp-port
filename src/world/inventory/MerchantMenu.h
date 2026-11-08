#pragma once

#include <memory>

#include "world/inventory/Container.h"
#include "world/inventory/IInventory.h"
#include "world/item/ItemInstance.h"

class InventoryPlayer;
class Level;
class Player;
class Merchant;
class MerchantRecipe;

// 1.5.2 TraderInventory: two payment slots and the result slot
class MerchantContainer : public IInventory
{
	Merchant &merchant;
	ItemInstance items[3];
	Player &player;
	MerchantRecipe *activeRecipe = nullptr;
	int_t selectionHint = 0;

	static bool isPaymentSlot(int_t slot) { return slot == 0 || slot == 1; }

public:
	MerchantContainer(Player &player, Merchant &merchant) : merchant(merchant), player(player) {}

	int_t getSizeInventory() const override { return 3; }
	ItemInstance *getStackInSlot(int_t slot) override { return slot >= 0 && slot < 3 && !items[slot].isEmpty() ? &items[slot] : nullptr; }
	const ItemInstance *getStackInSlot(int_t slot) const override { return slot >= 0 && slot < 3 && !items[slot].isEmpty() ? &items[slot] : nullptr; }
	ItemInstance decrStackSize(int_t slot, int_t count) override;
	ItemInstance removeItemNoUpdate(int_t slot);
	void setInventorySlotContents(int_t slot, const ItemInstance &item) override;
	jstring getInvName() const override { return u"mob.villager"; }
	void onInventoryChanged() override { updateSellItem(); }
	bool canInteractWith(Player &who) const override;

	void updateSellItem();
	MerchantRecipe *getActiveRecipe() const { return activeRecipe; }
	void setSelectionHint(int_t hint) { selectionHint = hint; updateSellItem(); }
};

// 1.5.2 TraderMenu
class MerchantMenu : public Container
{
	Merchant &merchant;
	Level &level;

public:
	MerchantContainer tradeContainer;

	MerchantMenu(InventoryPlayer &inventory, Merchant &merchant, Level &level, Player &player);
	void setSelectionHint(int_t hint) { tradeContainer.setSelectionHint(hint); }
	bool canUse(Player &player) const override;
	void onCraftMatrixChanged(IInventory &inventory) override;
	std::unique_ptr<ItemInstance> quickMoveStack(int_t slot) override;
	void onClosed(Player &player) override;
	Merchant &getMerchant() { return merchant; }
};
