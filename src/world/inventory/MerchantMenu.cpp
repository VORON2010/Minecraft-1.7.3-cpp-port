#include "world/inventory/MerchantMenu.h"

#include "world/entity/npc/Merchant.h"
#include "world/entity/player/InventoryPlayer.h"
#include "world/entity/player/Player.h"
#include "world/inventory/Slot.h"
#include "world/level/Level.h"

// ---- MerchantContainer ----

ItemInstance MerchantContainer::decrStackSize(int_t slot, int_t count)
{
	if (slot < 0 || slot >= 3 || items[slot].isEmpty())
		return ItemInstance();
	if (slot == 2)
	{
		ItemInstance taken = items[slot];
		items[slot] = ItemInstance();
		return taken;
	}
	if (items[slot].stackSize <= count)
	{
		ItemInstance taken = items[slot];
		items[slot] = ItemInstance();
		if (isPaymentSlot(slot))
			updateSellItem();
		return taken;
	}
	ItemInstance taken = items[slot].remove(count);
	if (items[slot].stackSize == 0)
		items[slot] = ItemInstance();
	if (isPaymentSlot(slot))
		updateSellItem();
	return taken;
}

ItemInstance MerchantContainer::removeItemNoUpdate(int_t slot)
{
	ItemInstance taken = items[slot];
	items[slot] = ItemInstance();
	return taken;
}

void MerchantContainer::setInventorySlotContents(int_t slot, const ItemInstance &item)
{
	if (slot < 0 || slot >= 3)
		return;
	items[slot] = item;
	if (!item.isEmpty() && items[slot].stackSize > getInventoryStackLimit())
		items[slot].stackSize = getInventoryStackLimit();
	if (isPaymentSlot(slot))
		updateSellItem();
}

bool MerchantContainer::canInteractWith(Player &who) const
{
	return merchant.getTradingPlayer() == &who;
}

void MerchantContainer::updateSellItem()
{
	activeRecipe = nullptr;
	const ItemInstance *buyA = getStackInSlot(0);
	const ItemInstance *buyB = getStackInSlot(1);
	if (buyA == nullptr)
	{
		buyA = buyB;
		buyB = nullptr;
	}
	if (buyA == nullptr)
	{
		setInventorySlotContents(2, ItemInstance());
		return;
	}
	MerchantRecipeList *offers = merchant.getOffers(player);
	if (offers == nullptr)
		return;
	MerchantRecipe *recipe = offers->getRecipeFor(*buyA, buyB, selectionHint);
	if (recipe != nullptr && !recipe->isDeprecated())
	{
		activeRecipe = recipe;
		setInventorySlotContents(2, recipe->getSellItem());
	}
	else if (buyB != nullptr)
	{
		recipe = offers->getRecipeFor(*buyB, buyA, selectionHint);
		if (recipe != nullptr && !recipe->isDeprecated())
		{
			activeRecipe = recipe;
			setInventorySlotContents(2, recipe->getSellItem());
		}
		else
			setInventorySlotContents(2, ItemInstance());
	}
	else
		setInventorySlotContents(2, ItemInstance());
}

// ---- MerchantResultSlot (1.5.2 TradeResultSlot) ----

namespace
{
	class MerchantResultSlot : public Slot
	{
		MerchantContainer &slots;
		Merchant &merchant;

		static bool removePaymentItemsIfMatching(MerchantRecipe &recipe, ItemInstance *a, ItemInstance *b)
		{
			const ItemInstance &buyA = recipe.getBuyA();
			const ItemInstance *buyB = recipe.getBuyB();
			if (a == nullptr || a->itemID != buyA.itemID)
				return false;
			if (buyB != nullptr && b != nullptr && buyB->itemID == b->itemID)
			{
				a->stackSize -= buyA.stackSize;
				b->stackSize -= buyB->stackSize;
				return true;
			}
			if (buyB == nullptr && b == nullptr)
			{
				a->stackSize -= buyA.stackSize;
				return true;
			}
			return false;
		}

	public:
		MerchantResultSlot(Merchant &merchant, MerchantContainer &slots, int_t index, int_t x, int_t y)
			: Slot(slots, index, x, y), slots(slots), merchant(merchant) {}

		bool mayPlace(const ItemInstance &item) const override { (void)item; return false; }

		void onPickup(ItemInstance &item) override
		{
			(void)item;
			MerchantRecipe *recipe = slots.getActiveRecipe();
			if (recipe == nullptr)
				return;
			ItemInstance a = slots.getStackInSlot(0) != nullptr ? *slots.getStackInSlot(0) : ItemInstance();
			ItemInstance b = slots.getStackInSlot(1) != nullptr ? *slots.getStackInSlot(1) : ItemInstance();
			ItemInstance *pa = a.isEmpty() ? nullptr : &a;
			ItemInstance *pb = b.isEmpty() ? nullptr : &b;
			if (removePaymentItemsIfMatching(*recipe, pa, pb) || removePaymentItemsIfMatching(*recipe, pb, pa))
			{
				slots.setInventorySlotContents(0, a.stackSize > 0 ? a : ItemInstance());
				slots.setInventorySlotContents(1, b.stackSize > 0 ? b : ItemInstance());
				merchant.notifyTrade(*recipe);
			}
		}
	};
}

// ---- MerchantMenu ----

MerchantMenu::MerchantMenu(InventoryPlayer &inventory, Merchant &merchant, Level &level, Player &player)
	: merchant(merchant), level(level), tradeContainer(player, merchant)
{
	addSlot(std::make_unique<Slot>(tradeContainer, 0, 36, 53));
	addSlot(std::make_unique<Slot>(tradeContainer, 1, 62, 53));
	addSlot(std::make_unique<MerchantResultSlot>(merchant, tradeContainer, 2, 120, 53));
	for (int_t row = 0; row < 3; ++row)
		for (int_t column = 0; column < 9; ++column)
			addSlot(std::make_unique<Slot>(inventory, column + row * 9 + 9, 8 + column * 18, 84 + row * 18));
	for (int_t column = 0; column < 9; ++column)
		addSlot(std::make_unique<Slot>(inventory, column, 8 + column * 18, 142));
}

bool MerchantMenu::canUse(Player &player) const
{
	return merchant.getTradingPlayer() == &player;
}

void MerchantMenu::onCraftMatrixChanged(IInventory &inventory)
{
	tradeContainer.updateSellItem();
	Container::onCraftMatrixChanged(inventory);
}

std::unique_ptr<ItemInstance> MerchantMenu::quickMoveStack(int_t slot)
{
	if (slot == 2)
		return quickMoveStackTo(slot, 3, 39, true);
	if (slot != 0 && slot != 1)
	{
		if (slot >= 3 && slot < 30)
			return quickMoveStackTo(slot, 30, 39, false);
		if (slot >= 30 && slot < 39)
			return quickMoveStackTo(slot, 3, 30, false);
		return nullptr;
	}
	return quickMoveStackTo(slot, 3, 39, false);
}

void MerchantMenu::onClosed(Player &player)
{
	Container::onClosed(player);
	merchant.setTradingPlayer(nullptr);
	if (level.isOnline)
		return;
	for (int_t i = 0; i < 2; ++i)
	{
		ItemInstance dropped = tradeContainer.removeItemNoUpdate(i);
		if (!dropped.isEmpty())
			player.drop(dropped);
	}
}
