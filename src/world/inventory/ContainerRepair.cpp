#include "world/inventory/ContainerRepair.h"

#include <algorithm>
#include <iostream>
#include <map>

#include "world/entity/player/InventoryPlayer.h"
#include "world/entity/player/Player.h"
#include "world/inventory/Slot.h"
#include "world/item/Item.h"
#include "world/item/Items.h"
#include "world/item/enchantment/Enchantment.h"
#include "world/level/Level.h"
#include "world/level/tile/AnvilTile.h"
#include "world/level/tile/Tile.h"
#include "world/stats/StatCollector.h"

ItemInstance RepairInputInventory::decrStackSize(int_t slot, int_t count)
{
	if (slot < 0 || slot >= 2 || items[slot].isEmpty())
		return ItemInstance();
	ItemInstance taken = items[slot].remove(count);
	if (items[slot].stackSize <= 0)
		items[slot] = ItemInstance();
	onInventoryChanged();
	return taken;
}

void RepairInputInventory::setInventorySlotContents(int_t slot, const ItemInstance &stack)
{
	if (slot < 0 || slot >= 2)
		return;
	items[slot] = stack;
	onInventoryChanged();
}

void RepairInputInventory::onInventoryChanged()
{
	menu.createResult();
}

ItemInstance RepairResultInventory::decrStackSize(int_t slot, int_t count)
{
	(void)count;
	if (slot != 0 || item.isEmpty())
		return ItemInstance();
	ItemInstance taken = item;
	item = ItemInstance();
	return taken;
}

namespace
{
	class ResultSlot : public Slot
	{
		ContainerRepair &menu;

	public:
		ResultSlot(ContainerRepair &menu, IInventory &inventory, int_t slot, int_t x, int_t y) : Slot(inventory, slot, x, y), menu(menu) {}
		bool mayPlace(const ItemInstance &item) const override { (void)item; return false; }
		bool mayPickup(Player &player) const override
		{
			return (player.abilities.instabuild || player.experienceLevel >= menu.cost) && menu.cost > 0 && hasItem();
		}
		void onPickup(ItemInstance &item) override
		{
			(void)item;
			Player *taker = menu.lastPlayer;
			if (taker != nullptr)
				menu.onTake(*taker);
		}
	};

	using EnchantMap = std::map<int_t, int_t>;

	EnchantMap toMap(const std::vector<std::pair<short_t, short_t>> &list)
	{
		EnchantMap m;
		for (const auto &e : list)
			m[e.first] = e.second;
		return m;
	}

	// 1.4.7 EnchantmentHelper.getEnchantments: books keep theirs in StoredEnchantments
	EnchantMap enchantmentsOf(const ItemInstance &stack)
	{
		return toMap(stack.itemID == Items::enchantedBook->getShiftedIndex() ? stack.storedEnchantments : stack.enchantments);
	}

	void setEnchantments(const EnchantMap &m, ItemInstance &stack)
	{
		std::vector<std::pair<short_t, short_t>> list;
		for (const auto &kv : m)
			list.emplace_back(static_cast<short_t>(kv.first), static_cast<short_t>(kv.second));
		if (stack.itemID == Items::enchantedBook->getShiftedIndex())
			stack.storedEnchantments = list;
		else
			stack.enchantments = list;
	}

	int_t weightCost(const Enchantment &e)
	{
		switch (e.weight)
		{
		case 1: return 8;
		case 2: return 4;
		case 5: return 2;
		case 10: return 1;
		default: return 0;
		}
	}

	jstring hoverName(const ItemInstance &stack)
	{
		if (!stack.customName.empty())
			return stack.customName;
		Item *item = stack.getItem();
		jstring key;
		if (item != nullptr)
		{
			jstring custom = item->getHoverName(stack);
			if (!custom.empty())
				return custom;
			key = item->getDescriptionId(stack);
		}
		else if (Tile::tiles[stack.itemID] != nullptr)
			key = Tile::tiles[stack.itemID]->descriptionId;
		return StatCollector::translate(key + u".name");
	}

	bool equalsIgnoreCase(const jstring &a, const jstring &b)
	{
		if (a.size() != b.size())
			return false;
		for (size_t i = 0; i < a.size(); ++i)
		{
			char16_t x = a[i], y = b[i];
			if (x >= u'A' && x <= u'Z')
				x = static_cast<char16_t>(x - u'A' + u'a');
			if (y >= u'A' && y <= u'Z')
				y = static_cast<char16_t>(y - u'A' + u'a');
			if (x != y)
				return false;
		}
		return true;
	}
}

ContainerRepair::ContainerRepair(InventoryPlayer &inventory, Level &level, int_t x, int_t y, int_t z, Player &player)
	: level(level), x(x), y(y), z(z), player(player), inputs(*this)
{
	lastPlayer = &player;
	addSlot(std::make_unique<Slot>(inputs, 0, 27, 47));
	addSlot(std::make_unique<Slot>(inputs, 1, 76, 47));
	addSlot(std::make_unique<ResultSlot>(*this, result, 0, 134, 47));
	for (int_t row = 0; row < 3; ++row)
		for (int_t column = 0; column < 9; ++column)
			addSlot(std::make_unique<Slot>(inventory, column + row * 9 + 9, 8 + column * 18, 84 + row * 18));
	for (int_t column = 0; column < 9; ++column)
		addSlot(std::make_unique<Slot>(inventory, column, 8 + column * 18, 142));
}

void ContainerRepair::createResult()
{
	const ItemInstance *input = inputs.getStackInSlot(0);
	cost = 0;
	int_t price = 0;
	int_t tax = 0;
	int_t namingCost = 0;
	if (input == nullptr)
	{
		result.setInventorySlotContents(0, ItemInstance());
		return;
	}
	ItemInstance out = *input;
	const ItemInstance *addition = inputs.getStackInSlot(1);
	EnchantMap enchants = enchantmentsOf(out);
	bool book = false;
	tax += input->repairCost + (addition == nullptr ? 0 : addition->repairCost);
	repairItemCountCost = 0;
	Item *outItem = out.getItem();
	bool damageable = outItem != nullptr && outItem->getMaxDamage() > 0;
	if (addition != nullptr)
	{
		book = addition->itemID == Items::enchantedBook->getShiftedIndex() && !addition->storedEnchantments.empty();
		if (damageable && outItem->isValidRepairItem(*input, *addition))
		{
			int_t fix = std::min<int_t>(out.itemDamage, out.getMaxDamage() / 4);
			if (fix <= 0)
			{
				result.setInventorySlotContents(0, ItemInstance());
				cost = 0;
				return;
			}
			int_t used = 0;
			for (; fix > 0 && used < addition->stackSize; ++used)
			{
				out.itemDamage = out.itemDamage - fix;
				price += std::max<int_t>(1, fix / 100) + static_cast<int_t>(enchants.size());
				fix = std::min<int_t>(out.itemDamage, out.getMaxDamage() / 4);
			}
			repairItemCountCost = used;
		}
		else
		{
			if (!book && (out.itemID != addition->itemID || !damageable))
			{
				result.setInventorySlotContents(0, ItemInstance());
				cost = 0;
				return;
			}
			if (damageable && !book)
			{
				int_t left = input->getMaxDamage() - input->itemDamage;
				int_t right = addition->getMaxDamage() - addition->itemDamage;
				int_t bonus = right + out.getMaxDamage() * 12 / 100;
				int_t total = left + bonus;
				int_t damage = out.getMaxDamage() - total;
				if (damage < 0)
					damage = 0;
				if (damage < out.itemDamage)
				{
					out.itemDamage = damage;
					price += std::max<int_t>(1, bonus / 100);
				}
			}
			EnchantMap other = enchantmentsOf(*addition);
			for (const auto &kv : other)
			{
				const Enchantment *e = Enchantment::enchantments[kv.first];
				if (e == nullptr)
					continue;
				int_t current = enchants.count(kv.first) ? enchants[kv.first] : 0;
				int_t level = kv.second;
				level = current == level ? level + 1 : std::max(level, current);
				int_t gained = level - current;
				bool ok = outItem != nullptr && canEnchant(e->category, *outItem);
				if (player.abilities.instabuild)
					ok = true;
				for (const auto &mine : enchants)
				{
					if (mine.first != kv.first && Enchantment::enchantments[mine.first] != nullptr && !e->isCompatibleWith(*Enchantment::enchantments[mine.first]))
					{
						ok = false;
						price += gained;
					}
				}
				if (ok)
				{
					if (level > e->getMaxLevel())
						level = e->getMaxLevel();
					enchants[kv.first] = level;
					int_t c = weightCost(*e);
					if (book)
						c = std::max<int_t>(1, c / 2);
					price += c * gained;
				}
			}
		}
	}

	if (hasName && !itemName.empty() && !equalsIgnoreCase(itemName, hoverName(*input)))
	{
		namingCost = damageable ? 7 : input->stackSize * 5;
		price += namingCost;
		if (!input->customName.empty())
			tax += namingCost / 2;
		out.customName = itemName;
	}

	int_t count = 0;
	for (const auto &kv : enchants)
	{
		const Enchantment *e = Enchantment::enchantments[kv.first];
		if (e == nullptr)
			continue;
		count++;
		int_t c = weightCost(*e);
		if (book)
			c = std::max<int_t>(1, c / 2);
		tax += count + kv.second * c;
	}
	if (book)
		tax = std::max<int_t>(1, tax / 2);
	cost = tax + price;
	bool hasResult = price > 0;
	if (namingCost == price && namingCost > 0 && cost >= 40)
		cost = 39;
	if (cost >= 40 && !player.abilities.instabuild)
		hasResult = false;
	if (!hasResult)
	{
		result.setInventorySlotContents(0, ItemInstance());
		return;
	}
	int_t baseCost = out.repairCost;
	if (addition != nullptr && baseCost < addition->repairCost)
		baseCost = addition->repairCost;
	if (!out.customName.empty())
		baseCost -= 9;
	if (baseCost < 0)
		baseCost = 0;
	baseCost += 2;
	out.repairCost = baseCost;
	setEnchantments(enchants, out);
	result.setInventorySlotContents(0, out);
}

void ContainerRepair::setItemName(const jstring &name)
{
	itemName = name;
	hasName = true;
	if (ItemInstance *out = result.getStackInSlot(0))
		out->customName = name;
	createResult();
}

void ContainerRepair::onTake(Player &taker)
{
	// 1.4.7 AnvilMenu result slot onItemRemoved
	if (!taker.abilities.instabuild)
	{
		taker.experienceLevel -= cost;
		if (taker.experienceLevel < 0)
			taker.experienceLevel = 0;
	}
	int_t used = repairItemCountCost;
	inputs.setInventorySlotContents(0, ItemInstance());
	if (used > 0)
	{
		const ItemInstance *addition = inputs.getStackInSlot(1);
		if (addition != nullptr && addition->stackSize > used)
		{
			ItemInstance rest = *addition;
			rest.stackSize -= used;
			inputs.setInventorySlotContents(1, rest);
		}
		else
			inputs.setInventorySlotContents(1, ItemInstance());
	}
	else
		inputs.setInventorySlotContents(1, ItemInstance());
	cost = 0;
	if (!taker.abilities.instabuild && !level.isOnline && level.getTile(x, y, z) == Tile::anvil.id && level.random.nextFloat() < 0.12f)
	{
		int_t data = level.getData(x, y, z);
		int_t dir = data & 3;
		int_t wear = (data >> 2) + 1;
		if (wear > 2)
		{
			level.setTile(x, y, z, 0);
			level.levelEvent(1020, x, y, z, 0);
		}
		else
		{
			level.setData(x, y, z, dir | wear << 2);
			level.levelEvent(1021, x, y, z, 0);
		}
	}
	else if (!level.isOnline)
		level.levelEvent(1021, x, y, z, 0);
}

void ContainerRepair::onClosed(Player &taker)
{
	Container::onClosed(taker);
	if (level.isOnline)
		return;
	for (int_t i = 0; i < 2; ++i)
	{
		const ItemInstance *stack = inputs.getStackInSlot(i);
		if (stack == nullptr)
			continue;
		ItemInstance dropped = *stack;
		inputs.setInventorySlotContents(i, ItemInstance());
		taker.drop(dropped);
	}
}

bool ContainerRepair::canUse(Player &taker) const
{
	if (level.getTile(x, y, z) != Tile::anvil.id)
		return false;
	return taker.distanceToSqr(x + 0.5, y + 0.5, z + 0.5) <= 64.0;
}

std::unique_ptr<ItemInstance> ContainerRepair::quickMoveStack(int_t slot)
{
	if (slot == 2)
		return quickMoveStackTo(slot, 3, 39, true);
	if (slot != 0 && slot != 1)
	{
		if (slot >= 3 && slot < 39)
			return quickMoveStackTo(slot, 0, 2, false);
		return nullptr;
	}
	return quickMoveStackTo(slot, 3, 39, false);
}
