#include "world/level/tile/entity/ChestTileEntity.h"

#include "nbt/ListTag.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/ChestTile.h"

void ChestTileEntity::load(CompoundTag &tag)
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

void ChestTileEntity::save(CompoundTag &tag)
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

ItemInstance &ChestTileEntity::getItem(int_t slot)
{
	return items[slot];
}

const ItemInstance &ChestTileEntity::getItem(int_t slot) const
{
	return items[slot];
}

void ChestTileEntity::setItem(int_t slot, const ItemInstance &item)
{
	items[slot] = item;
	if (!items[slot].isEmpty() && items[slot].stackSize > items[slot].getMaxStackSize())
		items[slot].stackSize = items[slot].getMaxStackSize();
	setChanged();
}

ItemInstance ChestTileEntity::removeItem(int_t slot, int_t count)
{
	if (items[slot].isEmpty() || count <= 0)
		return ItemInstance();

	if (items[slot].stackSize <= count)
	{
		ItemInstance removed = items[slot];
		items[slot] = ItemInstance();
		setChanged();
		return removed;
	}

	ItemInstance removed = items[slot].remove(count);
	if (items[slot].isEmpty())
		items[slot] = ItemInstance();
	setChanged();
	return removed;
}

int_t ChestTileEntity::getContainerSize() const
{
	return static_cast<int_t>(items.size());
}

bool ChestTileEntity::canUse(Player &player) const
{
	if (level == nullptr)
		return false;

	auto tileEntity = level->getTileEntity(x, y, z);
	if (tileEntity.get() != this)
		return false;

	return player.distanceToSqr(static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5, static_cast<double>(z) + 0.5) <= 64.0;
}

jstring ChestTileEntity::getName() const
{
	return u"Chest";
}

void ChestTileEntity::clearCache()
{
	hasCheckedNeighbors = false;
}

// 1.4.7 ChestBlockEntity.updateShape
void ChestTileEntity::updateNeighbors()
{
	if (hasCheckedNeighbors || level == nullptr)
		return;
	hasCheckedNeighbors = true;
	auto chestAt = [this](int_t xx, int_t yy, int_t zz) -> ChestTileEntity * {
		if (level->getTile(xx, yy, zz) != Tile::chest.id)
			return nullptr;
		return dynamic_cast<ChestTileEntity *>(level->getTileEntity(xx, yy, zz).get());
	};
	westNeighbor = chestAt(x - 1, y, z);
	eastNeighbor = chestAt(x + 1, y, z);
	northNeighbor = chestAt(x, y, z - 1);
	southNeighbor = chestAt(x, y, z + 1);
	for (ChestTileEntity *n : {northNeighbor, southNeighbor, eastNeighbor, westNeighbor})
		if (n != nullptr)
			n->hasCheckedNeighbors = false;
}

// 1.4.7 ChestBlockEntity.tick: lid animation and the open/close sounds
void ChestTileEntity::tick()
{
	TileEntity::tick();
	updateNeighbors();
	tickCount++;
	oOpenness = openness;
	const float speed = 0.1f;
	if (openCount > 0 && openness == 0.0f && northNeighbor == nullptr && westNeighbor == nullptr)
	{
		double sx = x + 0.5;
		double sz = z + 0.5;
		if (southNeighbor != nullptr)
			sz += 0.5;
		if (eastNeighbor != nullptr)
			sx += 0.5;
		level->playSoundEffect(sx, y + 0.5, sz, u"random.chestopen", 0.5f, level->random.nextFloat() * 0.1f + 0.9f);
	}
	if ((openCount == 0 && openness > 0.0f) || (openCount > 0 && openness < 1.0f))
	{
		float before = openness;
		openness += openCount > 0 ? speed : -speed;
		if (openness > 1.0f)
			openness = 1.0f;
		if (openness < 0.5f && before >= 0.5f && northNeighbor == nullptr && westNeighbor == nullptr)
		{
			double sx = x + 0.5;
			double sz = z + 0.5;
			if (southNeighbor != nullptr)
				sz += 0.5;
			if (eastNeighbor != nullptr)
				sx += 0.5;
			level->playSoundEffect(sx, y + 0.5, sz, u"random.chestclosed", 0.5f, level->random.nextFloat() * 0.1f + 0.9f);
		}
		if (openness < 0.0f)
			openness = 0.0f;
	}
}

void ChestTileEntity::startOpen()
{
	openCount++;
}

void ChestTileEntity::stopOpen()
{
	if (openCount > 0)
		openCount--;
}
