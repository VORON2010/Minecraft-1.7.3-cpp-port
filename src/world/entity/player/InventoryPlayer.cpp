#include "world/entity/player/InventoryPlayer.h"

#include <utility>

#include "nbt/CompoundTag.h"
#include "nbt/ListTag.h"
#include "world/entity/Entity.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/item/Item.h"
#include "world/item/ItemArmor.h"

InventoryPlayer::InventoryPlayer(Player *player) : player(player)
{
}

ItemInstance *InventoryPlayer::getCurrentItem()
{
	if (currentItem >= 0 && currentItem < 9)
	{
		ItemInstance &item = mainInventory[currentItem];
		if (!item.isEmpty())
			return &item;
	}
	return nullptr;
}

ItemInstance *InventoryPlayer::getSelected()
{
	return getCurrentItem();
}

ItemInstance *InventoryPlayer::getItem(int_t slot)
{
	if (slot < 0 || slot >= getContainerSize())
		return nullptr;
	ItemInstance &item = slot < static_cast<int_t>(mainInventory.size())
		? mainInventory[slot]
		: armorInventory[slot - static_cast<int_t>(mainInventory.size())];
	return item.isEmpty() ? nullptr : &item;
}

const ItemInstance *InventoryPlayer::getItem(int_t slot) const
{
	if (slot < 0 || slot >= getContainerSize())
		return nullptr;
	const ItemInstance &item = slot < static_cast<int_t>(mainInventory.size())
		? mainInventory[slot]
		: armorInventory[slot - static_cast<int_t>(mainInventory.size())];
	return item.isEmpty() ? nullptr : &item;
}

ItemInstance InventoryPlayer::removeItem(int_t slot, int_t count)
{
	ItemInstance *item = getItem(slot);
	if (item == nullptr)
		return ItemInstance();
	ItemInstance removed;
	if (item->stackSize <= count)
	{
		removed = *item;
		setItem(slot, ItemInstance());
	}
	else
	{
		removed = item->remove(count);
		if (item->stackSize == 0)
			setItem(slot, ItemInstance());
	}
	return removed;
}

ItemInstance *InventoryPlayer::getCarried()
{
	if (!carried.isEmpty())
		return &carried;
	return nullptr;
}

void InventoryPlayer::setCarried(const ItemInstance &item)
{
	carried = item;
}

void InventoryPlayer::setCarried(ItemInstance &&item)
{
	carried = std::move(item);
}

void InventoryPlayer::setCarriedNull()
{
	carried = ItemInstance();
}

int_t InventoryPlayer::getContainerSize() const
{
	return static_cast<int_t>(mainInventory.size() + armorInventory.size());
}

jstring InventoryPlayer::getName() const
{
	return u"Inventory";
}

void InventoryPlayer::setChanged()
{
	inventoryChanged = true;
}

bool InventoryPlayer::canUse(Player &other) const
{
	return player != nullptr && !player->removed && other.distanceToSqr(*player) <= 64.0;
}

Player &InventoryPlayer::getPlayer() const
{
	return *player;
}


void InventoryPlayer::setItem(int_t slot, const ItemInstance &item)
{
	if (slot >= 0 && slot < static_cast<int_t>(mainInventory.size()))
		mainInventory[slot] = item;
	else if (slot >= static_cast<int_t>(mainInventory.size()) && slot < getContainerSize())
		armorInventory[slot - static_cast<int_t>(mainInventory.size())] = item;
}

void InventoryPlayer::setCurrentItem(int_t itemId)
{
	int_t slot = -1;
	for (int_t i = 0; i < static_cast<int_t>(mainInventory.size()); ++i)
	{
		if (!mainInventory[i].isEmpty() && mainInventory[i].itemID == itemId)
		{
			slot = i;
			break;
		}
	}

	if (slot >= 0 && slot < 9)
		currentItem = slot;
}

void InventoryPlayer::changeCurrentItem(int_t direction)
{
	if (direction > 0)
		direction = 1;
	else if (direction < 0)
		direction = -1;

	for (currentItem -= direction; currentItem < 0; currentItem += 9)
	{
	}
	while (currentItem >= 9)
		currentItem -= 9;
}

void InventoryPlayer::tick()
{
	for (ItemInstance &item : mainInventory)
		if (!item.isEmpty() && item.popTime > 0)
			item.popTime--;

	if (player != nullptr)
	{
		Level &level = player->level;
		for (int_t i = 0; i < static_cast<int_t>(mainInventory.size()); ++i)
		{
			ItemInstance &item = mainInventory[i];
			if (item.isEmpty())
				continue;
			Item *itemObj = item.getItem();
			if (itemObj == nullptr)
				continue;
			itemObj->onUpdate(item, level, *player, i, i == currentItem);
		}
	}
}

int_t InventoryPlayer::getFreeSlot() const
{
	for (int_t i = 0; i < static_cast<int_t>(mainInventory.size()); ++i)
		if (mainInventory[i].isEmpty())
			return i;
	return -1;
}

int_t InventoryPlayer::getSlotWithRemainingSpace(const ItemInstance &item) const
{
	for (int_t i = 0; i < static_cast<int_t>(mainInventory.size()); ++i)
	{
		const ItemInstance &existing = mainInventory[i];
		if (!existing.isEmpty() && existing.itemID == item.itemID && existing.isStackable()
			&& existing.stackSize < existing.getMaxStackSize() && existing.stackSize < getInventoryStackLimit()
			&& (!existing.getHasSubtypes() || existing.itemDamage == item.itemDamage))
			return i;
	}
	return -1;
}

bool InventoryPlayer::add(ItemInstance &item)
{
	if (item.isItemDamaged())
	{
		int_t slot = getFreeSlot();
		if (slot < 0)
			return false;
		mainInventory[slot] = item;
		mainInventory[slot].popTime = 5;
		item.stackSize = 0;
		return true;
	}

	int_t previous;
	do
	{
		previous = item.stackSize;
		int_t slot = getSlotWithRemainingSpace(item);
		if (slot < 0)
			slot = getFreeSlot();
		if (slot < 0)
			break;

		ItemInstance &target = mainInventory[slot];
		if (target.isEmpty())
			target = ItemInstance(item.itemID, 0, item.itemDamage);
		int_t toMove = item.stackSize;
		if (toMove > target.getMaxStackSize() - target.stackSize)
			toMove = target.getMaxStackSize() - target.stackSize;
		if (toMove > getInventoryStackLimit() - target.stackSize)
			toMove = getInventoryStackLimit() - target.stackSize;
		if (toMove == 0)
			break;
		target.stackSize += toMove;
		target.popTime = 5;
		item.stackSize -= toMove;
	}
	while (item.stackSize > 0 && item.stackSize < previous);
	return item.stackSize < previous;
}

bool InventoryPlayer::consumeItem(int_t itemId)
{
	for (ItemInstance &item : mainInventory)
	{
		if (item.isEmpty() || item.itemID != itemId)
			continue;
		if (--item.stackSize <= 0)
			item = ItemInstance();
		return true;
	}
	return false;
}

void InventoryPlayer::dropAll()
{
	if (player == nullptr)
		return;

	for (ItemInstance &item : mainInventory)
	{
		if (!item.isEmpty())
		{
			player->drop(item, true);
		item = ItemInstance();
		}
	}

	for (ItemInstance &item : armorInventory)
	{
		if (!item.isEmpty())
		{
			player->drop(item, true);
			item = ItemInstance();
		}
	}
}

float InventoryPlayer::getDestroySpeed(Tile &tile)
{
	ItemInstance *item = getCurrentItem();
	if (item != nullptr)
		return item->getDestroySpeed(tile);
	return 1.0f;
}

bool InventoryPlayer::canDestroySpecial(Tile &tile)
{
	if (tile.material.isHarvestable())
		return true;
	ItemInstance *item = getCurrentItem();
	if (item != nullptr)
		return item->canDestroySpecial(tile);
	return false;
}

int_t InventoryPlayer::getAttackDamage(Entity &entity)
{
	ItemInstance *item = getCurrentItem();
	if (item != nullptr)
		return item->getAttackDamage(entity);
	return 1;
}

int_t InventoryPlayer::getArmorValue() const
{
	int_t totalReduce = 0;
	int_t totalCurrent = 0;
	int_t totalMax = 0;

	for (int_t i = 0; i < static_cast<int_t>(armorInventory.size()); ++i)
	{
		const ItemInstance &armor = armorInventory[i];
		if (armor.isEmpty())
			continue;
		Item *item = armor.getItem();
		if (item == nullptr)
			continue;

		auto *armorItem = dynamic_cast<ItemArmor*>(item);
		if (armorItem == nullptr)
			continue;

		int_t curDurability = armor.getMaxDamage() - armor.itemDamage;
		totalCurrent += curDurability;
		totalMax += armor.getMaxDamage();
		totalReduce += armorItem->damageReduceAmount;
	}

	if (totalMax == 0)
		return 0;

	return (totalReduce - 1) * totalCurrent / totalMax + 1;
}

void InventoryPlayer::hurtArmor(int_t damage)
{
	for (int_t i = 0; i < static_cast<int_t>(armorInventory.size()); ++i)
	{
		ItemInstance &armor = armorInventory[i];
		if (armor.isEmpty())
			continue;
		Item *item = armor.getItem();
		if (item == nullptr)
			continue;

		auto *armorItem = dynamic_cast<ItemArmor*>(item);
		if (armorItem == nullptr)
			continue;

		armor.damageItem(damage, *player);
		if (armor.stackSize == 0)
			armorInventory[i] = ItemInstance();
	}
}

void InventoryPlayer::save(ListTag &tag) const
{
	for (int_t i = 0; i < static_cast<int_t>(mainInventory.size()); ++i)
	{
		const ItemInstance &item = mainInventory[i];
		if (item.isEmpty())
			continue;
		auto itemTag = std::make_shared<CompoundTag>();
		itemTag->putByte(u"Slot", static_cast<byte_t>(i));
		item.save(*itemTag);
		tag.add(itemTag);
	}

	for (int_t i = 0; i < static_cast<int_t>(armorInventory.size()); ++i)
	{
		const ItemInstance &item = armorInventory[i];
		if (item.isEmpty())
			continue;
		auto itemTag = std::make_shared<CompoundTag>();
		itemTag->putByte(u"Slot", static_cast<byte_t>(i + 100));
		item.save(*itemTag);
		tag.add(itemTag);
	}
}

void InventoryPlayer::load(ListTag &tag)
{
	armorInventory.fill(ItemInstance());
	for (int_t i = 0; i < tag.size(); ++i)
	{
		auto itemTag = std::dynamic_pointer_cast<CompoundTag>(tag.get(i));
		if (!itemTag)
			continue;
		int_t slot = itemTag->getByte(u"Slot") & 255;
		ItemInstance item(*itemTag);
		if (item.isEmpty())
			continue;
		if (slot >= 0 && slot < static_cast<int_t>(mainInventory.size()))
			mainInventory[slot] = item;
		else if (slot >= 100 && slot < 100 + static_cast<int_t>(armorInventory.size()))
			armorInventory[slot - 100] = item;
	}
}
