#include "world/item/CreativeModeTab.h"

#include <initializer_list>
#include <unordered_map>

#include "world/entity/EntityIO.h"
#include "world/item/Item.h"
#include "world/item/Items.h"
#include "world/item/PotionItem.h"
#include "world/item/alchemy/PotionBrewing.h"
#include "world/item/enchantment/Enchantment.h"
#include "world/level/tile/AllTiles.h"
#include "world/level/tile/AnvilTile.h"
#include "world/level/tile/SlabTile.h"

namespace CreativeModeTab
{

namespace
{
	// generated from the setCreativeModeTab calls of 1.5.2 Block.java and Item.java (and their constructors)
	const std::unordered_map<int_t, int_t> &table()
	{
		static const std::unordered_map<int_t, int_t> tabs = {
#include "world/item/CreativeModeTabTable.inc"
			// WoolBlock sets its id and tab in its own constructor
			{35, BUILDING_BLOCKS},
			// the extra music discs of this port
			{2268, MISC}, {2269, MISC}, {2270, MISC}, {2271, MISC}, {2272, MISC},
		};
		return tabs;
	}

	bool exists(int_t id)
	{
		if (id > 0 && id < 256)
			return Tile::tiles[id] != nullptr;
		return id >= 256 && id < static_cast<int_t>(Item::items.size()) && Item::items[id] != nullptr;
	}

	void range(std::vector<ItemInstance> &out, int_t id, int_t from, int_t to)
	{
		for (int_t d = from; d <= to; ++d)
			out.emplace_back(id, 1, d);
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

	// 1.5.2 PotionItem.ITEM_STACKS: one potion per distinct effect list in insertion order;
	// a later data value with the same effects replaces the earlier one in place
	const std::vector<int_t> &potionValues()
	{
		static std::vector<int_t> values;
		if (!values.empty())
			return values;
		std::vector<std::vector<MobEffectInstance>> keys;
		for (int_t base = 0; base <= 15; ++base)
			for (int_t splash = 0; splash <= 1; ++splash)
			{
				int_t data = splash == 0 ? (base | 8192) : (base | 16384);
				for (int_t mod = 0; mod <= 2; ++mod)
				{
					int_t d = data;
					if (mod == 1)
						d |= 32;
					else if (mod == 2)
						d |= 64;
					std::vector<MobEffectInstance> effects = PotionBrewing::getEffects(d, false);
					if (effects.empty())
						continue;
					bool found = false;
					for (size_t k = 0; k < keys.size(); ++k)
						if (sameEffects(keys[k], effects))
						{
							values[k] = d;
							found = true;
							break;
						}
					if (!found)
					{
						keys.push_back(effects);
						values.push_back(d);
					}
				}
			}
		return values;
	}

	void addEnchantedBooks(std::vector<ItemInstance> &out, std::initializer_list<EnchantmentCategory> categories)
	{
		for (Enchantment *e : Enchantment::enchantments)
		{
			if (e == nullptr)
				continue;
			bool match = false;
			for (EnchantmentCategory c : categories)
				match |= e->category == c;
			if (!match)
				continue;
			ItemInstance book(Items::enchantedBook->getShiftedIndex(), 1, 0);
			EnchantedBookItem::addEnchantment(book, e->id, e->getMaxLevel());
			out.push_back(book);
		}
	}
}

int_t getTab(int_t id)
{
	auto it = table().find(id);
	return it != table().end() ? it->second : -1;
}

jstring getName(int_t tab)
{
	static const char16_t *names[] = {u"buildingBlocks", u"decorations", u"redstone", u"transportation", u"misc", u"search",
		u"food", u"tools", u"combat", u"brewing", u"materials", u"inventory"};
	return names[tab];
}

int_t getIconItem(int_t tab)
{
	switch (tab)
	{
	case BUILDING_BLOCKS: return Tile::brick.id;
	case DECORATIONS: return Tile::rose.id;
	case REDSTONE: return Items::redstone->getShiftedIndex();
	case TRANSPORTATION: return Tile::railPowered.id;
	case MISC: return Items::bucketLava->getShiftedIndex();
	case SEARCH: return Items::compass->getShiftedIndex();
	case FOOD: return Items::apple->getShiftedIndex();
	case TOOLS: return Items::axeIron->getShiftedIndex();
	case COMBAT: return Items::swordGold->getShiftedIndex();
	case BREWING: return Items::potion->getShiftedIndex();
	case MATERIALS: return Items::stick->getShiftedIndex();
	case INVENTORY: return Tile::chest.id;
	}
	return 1;
}

bool hasScrollbar(int_t tab) { return tab != INVENTORY; }
bool hasTooltips(int_t tab) { return tab != INVENTORY; }

const char16_t *getTexture(int_t tab)
{
	if (tab == SEARCH)
		return u"search.png";
	if (tab == INVENTORY)
		return u"survival_inv.png";
	return u"list_items.png";
}

void addSubtypes(int_t id, std::vector<ItemInstance> &out)
{
	if (id == Tile::wood.id || id == Tile::treeTrunk.id || id == Tile::leaves.id || id == Tile::sapling.id || id == Tile::stoneBrick.id)
		range(out, id, 0, 3);
	else if (id == Tile::wool.id)
		range(out, id, 0, 15);
	else if (id == Tile::sandstone.id || id == Tile::monsterEgg.id || id == Tile::anvil.id)
		range(out, id, 0, 2);
	else if (id == Tile::tallGrass.id)
		range(out, id, 1, 2);
	else if (id == Tile::slabSingle.id)
	{
		// 1.5.2 StoneSlabBlock: every kind but the old wooden one (2)
		for (int_t d = 0; d <= SlabTile::MAX_STONE_DATA; ++d)
			if (d != 2)
				out.emplace_back(id, 1, d);
	}
	else if (id == Items::coal->getShiftedIndex() || id == Items::appleGold->getShiftedIndex())
		range(out, id, 0, 1);
	else if (id == Items::dyePowder->getShiftedIndex())
		range(out, id, 0, 15);
	else if (id == Items::potion->getShiftedIndex())
	{
		out.emplace_back(id, 1, 0);
		for (int_t d : potionValues())
			out.emplace_back(id, 1, d);
	}
	else if (id == Items::spawnEgg->getShiftedIndex())
	{
		for (const EntityIO::SpawnEggData &egg : EntityIO::getSpawnEggs())
			out.emplace_back(id, 1, egg.id);
	}
	else
		out.emplace_back(id, 1, 0);
}

void addItems(int_t tab, std::vector<ItemInstance> &out)
{
	for (int_t id = 1; id < static_cast<int_t>(Item::items.size()); ++id)
		if (exists(id) && getTab(id) == tab)
			addSubtypes(id, out);
	if (tab == MISC)
		addEnchantedBooks(out, {EnchantmentCategory::ALL});
	else if (tab == TOOLS)
		addEnchantedBooks(out, {EnchantmentCategory::DIGGER});
	else if (tab == COMBAT)
		addEnchantedBooks(out, {EnchantmentCategory::ARMOR, EnchantmentCategory::ARMOR_FEET, EnchantmentCategory::ARMOR_HEAD,
			EnchantmentCategory::ARMOR_LEGS, EnchantmentCategory::ARMOR_TORSO, EnchantmentCategory::WEAPON});
}

void addAllItems(std::vector<ItemInstance> &out)
{
	for (int_t id = 1; id < static_cast<int_t>(Item::items.size()); ++id)
		if (exists(id) && getTab(id) >= 0)
			addSubtypes(id, out);
	// EnchantedBookItem.addToCreativeMenu: every level of every enchantment
	for (Enchantment *e : Enchantment::enchantments)
	{
		if (e == nullptr)
			continue;
		for (int_t lvl = e->getMinLevel(); lvl <= e->getMaxLevel(); ++lvl)
		{
			ItemInstance book(Items::enchantedBook->getShiftedIndex(), 1, 0);
			EnchantedBookItem::addEnchantment(book, e->id, lvl);
			out.push_back(book);
		}
	}
}

}
