#include "world/level/tile/entity/FurnaceTileEntity.h"

#include "world/entity/player/Player.h"
#include "world/item/Item.h"
#include "world/item/Items.h"
#include "world/item/crafting/FurnaceRecipes.h"
#include "world/level/Level.h"
#include "world/level/tile/FurnaceTile.h"
#include "world/level/tile/SaplingTile.h"
#include "world/level/tile/Tile.h"

#include "nbt/CompoundTag.h"
#include "nbt/ListTag.h"

void FurnaceTileEntity::load(CompoundTag &tag)
{
	TileEntity::load(tag);
	items = {};

	std::shared_ptr<ListTag> itemTags = tag.getList(u"Items");
	if (itemTags != nullptr)
	{
		for (int_t i = 0; i < itemTags->size(); ++i)
		{
			auto itemTag = std::static_pointer_cast<CompoundTag>(itemTags->get(i));
			if (!itemTag)
				continue;
			int_t slot = itemTag->getByte(u"Slot") & 255;
			if (slot < 0 || slot >= static_cast<int_t>(items.size()))
				continue;
			items[slot] = ItemInstance(*itemTag);
		}
	}

	burnTime = tag.getShort(u"BurnTime");
	cookTime = tag.getShort(u"CookTime");
	currentItemBurnTime = getBurnDuration(items[1]);
}

void FurnaceTileEntity::save(CompoundTag &tag)
{
	TileEntity::save(tag);
	tag.putShort(u"BurnTime", static_cast<short_t>(burnTime));
	tag.putShort(u"CookTime", static_cast<short_t>(cookTime));

	auto itemTags = Util::make_shared<ListTag>();
	for (int_t i = 0; i < static_cast<int_t>(items.size()); ++i)
	{
		if (items[i].isEmpty())
			continue;
		auto itemTag = Util::make_shared<CompoundTag>();
		itemTag->putByte(u"Slot", static_cast<byte_t>(i));
		items[i].save(*itemTag);
		itemTags->add(itemTag);
	}
	tag.put(u"Items", itemTags);
}

void FurnaceTileEntity::tick()
{
	if (level == nullptr)
		return;

	bool wasBurning = burnTime > 0;
	bool changed = false;
	if (burnTime > 0)
		--burnTime;

	if (!level->isOnline)
	{
		if (burnTime == 0 && canSmelt())
		{
			currentItemBurnTime = burnTime = getBurnDuration(items[1]);
			if (burnTime > 0)
			{
				changed = true;
				if (!items[1].isEmpty())
				{
					--items[1].stackSize;
					if (items[1].isEmpty())
						items[1] = ItemInstance();
				}
			}
		}

		if (isBurning() && canSmelt())
		{
			++cookTime;
			if (cookTime == 200)
			{
				cookTime = 0;
				smeltItem();
				changed = true;
			}
		}
		else
		{
			cookTime = 0;
		}

		if (wasBurning != isBurning())
		{
			changed = true;
			FurnaceTile::setLitState(isBurning(), *level, x, y, z);
		}
	}

	if (changed)
		setChanged();
}

int_t FurnaceTileEntity::getCookProgressScaled(int_t scale) const
{
	return cookTime * scale / 200;
}

int_t FurnaceTileEntity::getBurnTimeRemainingScaled(int_t scale) const
{
	int_t burnBase = currentItemBurnTime;
	if (burnBase == 0)
		burnBase = 200;
	return burnTime * scale / burnBase;
}

bool FurnaceTileEntity::isBurning() const
{
	return burnTime > 0;
}

bool FurnaceTileEntity::canUse(Player &player) const
{
	if (level == nullptr)
		return false;
	auto tileEntity = level->getTileEntity(x, y, z);
	if (tileEntity.get() != this)
		return false;
	return player.distanceToSqr(static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5, static_cast<double>(z) + 0.5) <= 64.0;
}

bool FurnaceTileEntity::canSmelt() const
{
	if (items[0].isEmpty())
		return false;
	ItemInstance result = FurnaceRecipes::getInstance().getResult(items[0]);
	if (result.isEmpty())
		return false;
	if (items[2].isEmpty())
		return true;
	if (!items[2].sameItem(result))
		return false;
	return items[2].stackSize < items[2].getMaxStackSize() && items[2].stackSize < result.getMaxStackSize();
}

void FurnaceTileEntity::smeltItem()
{
	if (!canSmelt())
		return;

	ItemInstance result = FurnaceRecipes::getInstance().getResult(items[0]);
	if (items[2].isEmpty())
		items[2] = result;
	else
		++items[2].stackSize;

	--items[0].stackSize;
	if (items[0].isEmpty())
		items[0] = ItemInstance();
}

int_t FurnaceTileEntity::getBurnDuration(const ItemInstance &stack) const
{
	if (stack.isEmpty())
		return 0;
	int_t id = stack.itemID;
	
	if (id < static_cast<int_t>(Tile::tiles.size()) && Tile::tiles[id] != nullptr)
	{
		if (&Tile::tiles[id]->material == &Material::wood)
			return 300;
	}
	
	if (Items::coal != nullptr && id == Items::coal->getShiftedIndex())
		return 1600;
	
	if (Items::stick != nullptr && id == Items::stick->getShiftedIndex())
		return 100;
	
	if (Items::bucketLava != nullptr && id == Items::bucketLava->getShiftedIndex())
		return 20000;
	
	if (id == Tile::sapling.id)
		return 100;
	return 0;

}

void FurnaceTileEntity::setItem(int_t slot, const ItemInstance &item)
{
	items[slot] = item;
	if (!items[slot].isEmpty() && items[slot].stackSize > getInventoryStackLimit())
		items[slot].stackSize = getInventoryStackLimit();
	setChanged();
}

ItemInstance FurnaceTileEntity::removeItem(int_t slot, int_t count)
{
	if (items[slot].isEmpty())
		return ItemInstance();
	if (items[slot].stackSize <= count)
	{
		ItemInstance removed = items[slot];
		items[slot] = ItemInstance();
		setChanged();
		return removed;
	}
	ItemInstance removed = items[slot].remove(count);
	if (items[slot].stackSize == 0)
		items[slot] = ItemInstance();
	setChanged();
	return removed;
}
