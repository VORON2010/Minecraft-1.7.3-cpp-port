#include "world/level/tile/entity/DispenserTileEntity.h"

#include "nbt/ListTag.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"

void DispenserTileEntity::load(CompoundTag &tag)
{
	TileEntity::load(tag);
	items = {};
	auto itemTags = tag.getList(u"Items");
	if (itemTags == nullptr)
		return;
	for (int_t i = 0; i < itemTags->size(); ++i)
	{
		auto entry = std::dynamic_pointer_cast<CompoundTag>(itemTags->get(i));
		if (entry == nullptr)
			continue;
		int_t slot = entry->getByte(u"Slot") & 255;
		if (slot >= 0 && slot < static_cast<int_t>(items.size()))
			items[slot] = ItemInstance(*entry);
	}
}

void DispenserTileEntity::save(CompoundTag &tag)
{
	TileEntity::save(tag);
	auto itemTags = Util::make_shared<ListTag>();
	for (int_t i = 0; i < static_cast<int_t>(items.size()); ++i)
	{
		if (items[i].isEmpty())
			continue;
		auto entry = Util::make_shared<CompoundTag>();
		entry->putByte(u"Slot", static_cast<byte_t>(i));
		items[i].save(*entry);
		itemTags->add(entry);
	}
	tag.put(u"Items", itemTags);
}

void DispenserTileEntity::setItem(int_t slot, const ItemInstance &item)
{
	items[slot] = item;
	setChanged();
}

ItemInstance DispenserTileEntity::removeItem(int_t slot, int_t count)
{
	if (items[slot].isEmpty() || count <= 0)
		return ItemInstance();
	ItemInstance removed = items[slot].remove(count);
	if (!removed.isEmpty())
		setChanged();
	return removed;
}

bool DispenserTileEntity::canUse(Player &player) const
{
	if (level == nullptr)
		return false;
	auto tileEntity = level->getTileEntity(x, y, z);
	if (tileEntity.get() != this)
		return false;
	return player.distanceToSqr(static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5, static_cast<double>(z) + 0.5) <= 64.0;
}