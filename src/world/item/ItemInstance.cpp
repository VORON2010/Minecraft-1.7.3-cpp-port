#include "world/item/ItemInstance.h"

#include "Facing.h"
#include "nbt/CompoundTag.h"
#include "world/entity/Entity.h"
#include "world/entity/Mob.h"
#include "world/entity/player/Player.h"
#include "world/item/Item.h"
#include "world/level/tile/Tile.h"
#include "world/stats/StatList.h"

namespace
{

ItemInstanceReference::Snapshot snapshot(const ItemInstance &item)
{
	return {
		item.stackSize.load(std::memory_order_relaxed),
		item.itemID.load(std::memory_order_relaxed),
		item.itemDamage.load(std::memory_order_relaxed)
	};
}

}

ItemInstanceReference::ItemInstanceReference(const ItemInstance &item)
	: liveItem(&item), detachedItem(snapshot(item))
{
}

ItemInstanceReference::Snapshot ItemInstanceReference::get() const
{
	std::lock_guard<std::mutex> guard(lock);
	return liveItem == nullptr ? detachedItem : snapshot(*liveItem);
}

void ItemInstanceReference::detach(const ItemInstance &item)
{
	std::lock_guard<std::mutex> guard(lock);
	if (liveItem == &item)
	{
		detachedItem = snapshot(item);
		liveItem = nullptr;
	}
}

void ItemInstanceReference::move(const ItemInstance &from, const ItemInstance &to)
{
	std::lock_guard<std::mutex> guard(lock);
	if (liveItem == &from)
		liveItem = &to;
}

ItemInstance::ItemInstance(const ItemInstance &other)
	: stackSize(other.stackSize.load(std::memory_order_relaxed)),
	  itemID(other.itemID.load(std::memory_order_relaxed)),
	  itemDamage(other.itemDamage.load(std::memory_order_relaxed)), popTime(other.popTime)
{
}

ItemInstance::ItemInstance(ItemInstance &&other) noexcept
	: stackSize(other.stackSize.load(std::memory_order_relaxed)),
	  itemID(other.itemID.load(std::memory_order_relaxed)),
	  itemDamage(other.itemDamage.load(std::memory_order_relaxed)), popTime(other.popTime)
{
	moveJavaReference(other);
}

ItemInstance &ItemInstance::operator=(const ItemInstance &other)
{
	if (this == &other)
		return *this;
	detachJavaReference();
	stackSize.store(other.stackSize.load(std::memory_order_relaxed), std::memory_order_relaxed);
	itemID.store(other.itemID.load(std::memory_order_relaxed), std::memory_order_relaxed);
	itemDamage.store(other.itemDamage.load(std::memory_order_relaxed), std::memory_order_relaxed);
	popTime = other.popTime;
	return *this;
}

ItemInstance &ItemInstance::operator=(ItemInstance &&other) noexcept
{
	if (this == &other)
		return *this;
	detachJavaReference();
	stackSize.store(other.stackSize.load(std::memory_order_relaxed), std::memory_order_relaxed);
	itemID.store(other.itemID.load(std::memory_order_relaxed), std::memory_order_relaxed);
	itemDamage.store(other.itemDamage.load(std::memory_order_relaxed), std::memory_order_relaxed);
	popTime = other.popTime;
	moveJavaReference(other);
	return *this;
}

ItemInstance::~ItemInstance()
{
	detachJavaReference();
}

void ItemInstance::detachJavaReference()
{
	if (javaReference)
	{
		javaReference->detach(*this);
		javaReference.reset();
	}
}

void ItemInstance::moveJavaReference(ItemInstance &other)
{
	javaReference = std::move(other.javaReference);
	if (javaReference)
		javaReference->move(other, *this);
}

std::shared_ptr<const ItemInstanceReference> ItemInstance::retainReference() const
{
	if (!javaReference)
		javaReference = std::shared_ptr<ItemInstanceReference>(new ItemInstanceReference(*this));
	return javaReference;
}

ItemInstance::ItemInstance(int_t itemID) : ItemInstance(itemID, 1, 0)
{
}

ItemInstance::ItemInstance(int_t itemID, int_t count) : ItemInstance(itemID, count, 0)
{
}

ItemInstance::ItemInstance(int_t itemID, int_t count, int_t damage)
	: stackSize(count), itemID(itemID), itemDamage(damage)
{
}

ItemInstance::ItemInstance(CompoundTag &tag)
{
	load(tag);
}

Item *ItemInstance::getItem() const
{
	if (itemID < 0 || itemID >= static_cast<int_t>(Item::items.size()))
		return nullptr;
	return Item::items[itemID];
}

int_t ItemInstance::getMaxStackSize() const
{
	Item *item = getItem();
	if (item != nullptr)
		return item->getMaxStackSize();
	return 64;
}

int_t ItemInstance::getMaxDamage() const
{
	Item *item = getItem();
	if (item != nullptr)
		return item->getMaxDamage();
	return 0;
}

bool ItemInstance::getHasSubtypes() const
{
	return itemID >= 0 && itemID < static_cast<int_t>(Item::subtypeItems.size())
		&& Item::subtypeItems[itemID];
}

bool ItemInstance::isStackable() const
{
	return getMaxStackSize() > 1 && !isItemDamaged();
}

bool ItemInstance::isItemDamaged() const
{
	return getMaxDamage() > 0 && itemDamage > 0;
}

int_t ItemInstance::getIcon() const
{
	if (itemID >= 0 && itemID < static_cast<int_t>(Tile::tiles.size()) && Tile::tiles[itemID] != nullptr)
		return Tile::tiles[itemID]->getTexture(Facing::NORTH, itemDamage);
	Item *item = getItem();
	if (item != nullptr)
		return item->getIcon(*this);
	return 0;
}

float ItemInstance::getDestroySpeed(Tile &tile) const
{
	Item *item = getItem();
	if (item != nullptr)
		return item->getDestroySpeed(*this, tile);
	return 1.0f;
}

bool ItemInstance::canDestroySpecial(Tile &tile) const
{
	Item *item = getItem();
	if (item != nullptr)
		return item->canDestroySpecial(*this, tile);
	return true;
}

int_t ItemInstance::getAttackDamage(Entity &entity) const
{
	Item *item = getItem();
	if (item != nullptr)
		return item->getAttackDamage(*this, entity);
	return 1;
}

void ItemInstance::use(Level &level, Player &player)
{
	Item *item = getItem();
	if (item == nullptr)
		return;
	item->use(*this, level, player);
}

bool ItemInstance::useOn(Player &player, Level &level, int_t x, int_t y, int_t z, Facing face)
{
	Item *item = getItem();
	if (item == nullptr)
		return false;
	bool used = item->useOn(*this, player, level, x, y, z, face);
	if (used)
	{
		if (StatBase *stat = StatList::useItemStats[itemID])
			player.addStat(*stat, 1);
	}
	return used;
}

void ItemInstance::damageItem(int_t amount, Entity &entity)
{
	int_t maxDamage = getMaxDamage();
	if (maxDamage <= 0)
		return;
	itemDamage += amount;
	if (itemDamage > maxDamage)
	{
		if (auto *player = dynamic_cast<Player *>(&entity))
		{
			if (StatBase *stat = StatList::breakItemStats[itemID])
				player->addStat(*stat, 1);
		}
		stackSize--;
		if (stackSize < 0)
			stackSize = 0;
		itemDamage = 0;
	}
}

bool ItemInstance::hurtEnemy(Entity &target, Entity &attacker)
{
	Item *item = getItem();
	if (item == nullptr)
		return false;
	bool used = item->hurtEnemy(*this, target, attacker);
	if (used)
	{
		if (auto *player = dynamic_cast<Player *>(&attacker))
		{
			if (StatBase *stat = StatList::useItemStats[itemID])
				player->addStat(*stat, 1);
		}
	}
	return used;
}

void ItemInstance::saddleEntity(Mob &target)
{
	Item *item = getItem();
	if (item == nullptr)
		return;
	item->saddleEntity(*this, target);
}

bool ItemInstance::mineBlock(int_t tile, int_t x, int_t y, int_t z, Entity &miner)
{
	Item *item = getItem();
	if (item == nullptr)
		return false;
	bool used = item->mineBlock(*this, tile, x, y, z, miner);
	if (used)
	{
		if (auto *player = dynamic_cast<Player *>(&miner))
		{
			if (StatBase *stat = StatList::useItemStats[itemID])
				player->addStat(*stat, 1);
		}
	}
	return used;
}

void ItemInstance::onCrafted(Level &level, Player &player)
{
	if (StatBase *stat = StatList::craftItemStats[itemID])
		player.addStat(*stat, stackSize);
	Item *item = getItem();
	if (item != nullptr)
		item->onCreated(*this, level, player);
}

void ItemInstance::save(CompoundTag &tag) const
{
	tag.putShort(u"id", static_cast<short_t>(itemID));
	tag.putByte(u"Count", static_cast<byte_t>(stackSize));
	tag.putShort(u"Damage", static_cast<short_t>(itemDamage));
}

void ItemInstance::load(CompoundTag &tag)
{
	itemID = tag.getShort(u"id");
	stackSize = tag.getByte(u"Count");
	itemDamage = tag.getShort(u"Damage");
}

ItemInstance ItemInstance::remove(int_t count)
{
	stackSize -= count;
	return ItemInstance(itemID, count, itemDamage);
}

bool ItemInstance::sameItem(const ItemInstance &other) const
{
	return itemID == other.itemID && itemDamage == other.itemDamage;
}
