#include "world/level/tile/entity/BrewingStandTileEntity.h"

#include "nbt/CompoundTag.h"
#include "nbt/ListTag.h"
#include "world/entity/player/Player.h"
#include "world/item/Item.h"
#include "world/item/Items.h"
#include "world/item/PotionItem.h"
#include "world/item/alchemy/PotionBrewing.h"
#include "world/level/Level.h"

namespace
{
	const PotionItem &potionItem()
	{
		return *static_cast<const PotionItem *>(Items::potion);
	}

	bool sameEffects(const std::vector<MobEffectInstance> &a, const std::vector<MobEffectInstance> &b)
	{
		if (a.size() != b.size())
			return false;
		for (size_t i = 0; i < a.size(); ++i)
			if (a[i].getId() != b[i].getId() || a[i].getDuration() != b[i].getDuration() || a[i].getAmplifier() != b[i].getAmplifier())
				return false;
		return true;
	}

	// Java compares the cached effect lists by reference: the same data value gives the same list,
	// and two "no effect" potions both give null
	bool sameList(int_t a, int_t b)
	{
		return a == b || (potionItem().getMobEffects(a).empty() && potionItem().getMobEffects(b).empty());
	}
}

void BrewingStandTileEntity::tick()
{
	if (brewTime > 0)
	{
		brewTime--;
		if (brewTime == 0)
		{
			doBrew();
			setChanged();
		}
		else if (!isBrewable())
		{
			brewTime = 0;
			setChanged();
		}
		else if (ingredientId != items[3].itemID)
		{
			brewTime = 0;
			setChanged();
		}
	}
	else if (isBrewable())
	{
		brewTime = 600;
		ingredientId = items[3].itemID;
	}

	int_t bits = getPotionBits();
	if (bits != lastPotionCount)
	{
		lastPotionCount = bits;
		if (level != nullptr)
			level->setData(x, y, z, bits);
	}
	TileEntity::tick();
}

bool BrewingStandTileEntity::isBrewable() const
{
	if (items[3].isEmpty())
		return false;
	Item *ingredient = items[3].getItem();
	if (ingredient == nullptr || !ingredient->hasPotionBrewingFormula())
		return false;
	for (int_t i = 0; i < 3; ++i)
	{
		if (items[i].isEmpty() || items[i].itemID != Items::potion->getShiftedIndex())
			continue;
		int_t from = items[i].itemDamage;
		int_t to = applyIngredient(from, items[3]);
		if (!PotionItem::isThrowable(from) && PotionItem::isThrowable(to))
			return true;
		const auto &a = potionItem().getMobEffects(from);
		const auto &b = potionItem().getMobEffects(to);
		bool aNull = a.empty(), bNull = b.empty();
		if ((from <= 0 || !sameList(from, to)) && (aNull || (!sameEffects(a, b) && !bNull)) && from != to)
			return true;
	}
	return false;
}

void BrewingStandTileEntity::doBrew()
{
	if (!isBrewable())
		return;
	ItemInstance &ingredient = items[3];
	for (int_t i = 0; i < 3; ++i)
	{
		if (items[i].isEmpty() || items[i].itemID != Items::potion->getShiftedIndex())
			continue;
		int_t from = items[i].itemDamage;
		int_t to = applyIngredient(from, ingredient);
		const auto &a = potionItem().getMobEffects(from);
		const auto &b = potionItem().getMobEffects(to);
		bool aNull = a.empty(), bNull = b.empty();
		if ((from > 0 && sameList(from, to)) || (!aNull && (sameEffects(a, b) || bNull)))
		{
			if (!PotionItem::isThrowable(from) && PotionItem::isThrowable(to))
				items[i].itemDamage = to;
		}
		else if (from != to)
			items[i].itemDamage = to;
	}
	Item *item = ingredient.getItem();
	if (item != nullptr && item->hasContainerItem())
		items[3] = ItemInstance(item->getContainerItem()->getShiftedIndex(), 1, 0);
	else
	{
		items[3].stackSize--;
		if (items[3].stackSize <= 0)
			items[3] = ItemInstance();
	}
}

int_t BrewingStandTileEntity::applyIngredient(int_t data, const ItemInstance &ingredient) const
{
	if (ingredient.isEmpty())
		return data;
	Item *item = ingredient.getItem();
	if (item == nullptr || !item->hasPotionBrewingFormula())
		return data;
	return PotionBrewing::applyBrew(data, item->getPotionBrewingFormula());
}

int_t BrewingStandTileEntity::getPotionBits() const
{
	int_t bits = 0;
	for (int_t i = 0; i < 3; ++i)
		if (!items[i].isEmpty())
			bits |= 1 << i;
	return bits;
}

ItemInstance BrewingStandTileEntity::decrStackSize(int_t slot, int_t count)
{
	(void)count;
	if (slot < 0 || slot >= 4)
		return ItemInstance();
	ItemInstance taken = items[slot];
	items[slot] = ItemInstance();
	return taken;
}

void BrewingStandTileEntity::setInventorySlotContents(int_t slot, const ItemInstance &item)
{
	if (slot >= 0 && slot < 4)
		items[slot] = item;
}

bool BrewingStandTileEntity::canInteractWith(Player &player) const
{
	if (level == nullptr || level->getTileEntity(x, y, z).get() != this)
		return false;
	return player.distanceToSqr(x + 0.5, y + 0.5, z + 0.5) <= 64.0;
}

void BrewingStandTileEntity::load(CompoundTag &tag)
{
	TileEntity::load(tag);
	items = {};
	std::shared_ptr<ListTag> list = tag.getList(u"Items");
	for (int_t i = 0; list != nullptr && i < list->size(); ++i)
	{
		auto itemTag = std::dynamic_pointer_cast<CompoundTag>(list->get(i));
		if (itemTag == nullptr)
			continue;
		int_t slot = itemTag->getByte(u"Slot");
		if (slot >= 0 && slot < 4)
			items[slot] = ItemInstance(*itemTag);
	}
	brewTime = tag.getShort(u"BrewTime");
}

void BrewingStandTileEntity::save(CompoundTag &tag)
{
	TileEntity::save(tag);
	tag.putShort(u"BrewTime", static_cast<short_t>(brewTime));
	auto list = std::make_shared<ListTag>();
	for (int_t i = 0; i < 4; ++i)
	{
		if (items[i].isEmpty())
			continue;
		auto itemTag = std::make_shared<CompoundTag>();
		itemTag->putByte(u"Slot", static_cast<byte_t>(i));
		items[i].save(*itemTag);
		list->add(itemTag);
	}
	tag.put(u"Items", list);
}
