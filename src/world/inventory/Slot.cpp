#include "world/inventory/Slot.h"

#include "world/entity/player/Player.h"
#include "world/inventory/IInventory.h"
#include "world/item/Item.h"
#include "world/item/ItemArmor.h"
#include "world/item/ItemInstance.h"
#include "world/item/ItemPickaxe.h"
#include "world/item/Items.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/FurnaceTile.h"
#include "world/level/tile/PumpkinTile.h"
#include "world/level/tile/WorkbenchTile.h"
#include "world/stats/Achievement.h"
#include "world/stats/AchievementList.h"

Slot::Slot(IInventory &inventory, int_t slotIndex, int_t x, int_t y)
	: slotIndex(slotIndex), inventory(inventory), x(x), y(y)
{
}

void Slot::onPickup(ItemInstance &item)
{
	(void)item;
	setChanged();
}

bool Slot::mayPlace(const ItemInstance &item) const
{
	(void)item;
	return true;
}

ItemInstance *Slot::getItem()
{
	return inventory.getStackInSlot(slotIndex);
}

const ItemInstance *Slot::getItem() const
{
	return inventory.getStackInSlot(slotIndex);
}

bool Slot::hasItem() const
{
	return getItem() != nullptr;
}

void Slot::set(const ItemInstance *item)
{
	inventory.setInventorySlotContents(slotIndex, item == nullptr ? ItemInstance() : *item);
	setChanged();
}

void Slot::setChanged()
{
	inventory.onInventoryChanged();
}

int_t Slot::getMaxStackSize() const
{
	return inventory.getInventoryStackLimit();
}

int_t Slot::getBackgroundIcon() const
{
	return -1;
}

ItemInstance Slot::remove(int_t count)
{
	return inventory.decrStackSize(slotIndex, count);
}

ArmorSlot::ArmorSlot(IInventory &inventory, int_t slotIndex, int_t x, int_t y, int_t armorType)
	: Slot(inventory, slotIndex, x, y), armorType(armorType)
{
}

int_t ArmorSlot::getMaxStackSize() const
{
	return 1;
}

bool ArmorSlot::mayPlace(const ItemInstance &item) const
{
	if (auto *armor = dynamic_cast<ItemArmor *>(item.getItem()))
		return armor->armorType == armorType;
	return item.itemID == Tile::pumpkin.id && armorType == 0;
}

CraftingSlot::CraftingSlot(Player &player, IInventory &craftMatrix, IInventory &result,
	int_t slotIndex, int_t x, int_t y)
	: Slot(result, slotIndex, x, y), craftMatrix(craftMatrix), player(player)
{
}

bool CraftingSlot::mayPlace(const ItemInstance &item) const
{
	(void)item;
	return false;
}

void CraftingSlot::onPickup(ItemInstance &item)
{
	item.onCrafted(player.level, player);
	if (item.itemID == Tile::workBench.id)
		player.addStat(*AchievementList::buildWorkBench, 1);
	else if (item.itemID == Items::pickaxeWood->getShiftedIndex())
		player.addStat(*AchievementList::buildPickaxe, 1);
	else if (item.itemID == Tile::furnace.id)
		player.addStat(*AchievementList::buildFurnace, 1);
	else if (item.itemID == Items::hoeWood->getShiftedIndex())
		player.addStat(*AchievementList::buildHoe, 1);
	else if (item.itemID == Items::bread->getShiftedIndex())
		player.addStat(*AchievementList::makeBread, 1);
	else if (item.itemID == Items::cake->getShiftedIndex())
		player.addStat(*AchievementList::bakeCake, 1);
	else if (item.itemID == Items::pickaxeStone->getShiftedIndex())
		player.addStat(*AchievementList::buildBetterPickaxe, 1);
	else if (item.itemID == Items::swordWood->getShiftedIndex())
		player.addStat(*AchievementList::buildSword, 1);

	for (int_t slot = 0; slot < craftMatrix.getSizeInventory(); ++slot)
	{
		ItemInstance *ingredient = craftMatrix.getStackInSlot(slot);
		if (ingredient == nullptr)
			continue;
		Item *ingredientItem = ingredient->getItem();
		craftMatrix.decrStackSize(slot, 1);
		if (ingredientItem != nullptr && ingredientItem->hasContainerItem())
			craftMatrix.setInventorySlotContents(slot, ItemInstance(ingredientItem->getContainerItem()->getShiftedIndex()));
	}
}

FurnaceResultSlot::FurnaceResultSlot(Player &player, IInventory &inventory,
	int_t slotIndex, int_t x, int_t y)
	: Slot(inventory, slotIndex, x, y), player(player)
{
}

bool FurnaceResultSlot::mayPlace(const ItemInstance &item) const
{
	(void)item;
	return false;
}

void FurnaceResultSlot::onPickup(ItemInstance &item)
{
	item.onCrafted(player.level, player);
	if (item.itemID == Items::ingotIron->getShiftedIndex())
		player.addStat(*AchievementList::acquireIron, 1);
	if (item.itemID == Items::fishCooked->getShiftedIndex())
		player.addStat(*AchievementList::cookFish, 1);
	Slot::onPickup(item);
}
