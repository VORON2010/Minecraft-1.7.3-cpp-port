#include "world/item/enchantment/Enchantment.h"

#include "world/entity/Mob.h"
#include "world/entity/player/InventoryPlayer.h"
#include "world/item/ItemArmor.h"
#include "world/item/ItemInstance.h"
#include "world/item/ItemSword.h"
#include "world/item/ItemTool.h"
#include "world/stats/StatCollector.h"

std::array<Enchantment *, 256> Enchantment::enchantments = {};

Enchantment Enchantment::protection(0, 10, EnchantmentCategory::ARMOR, Kind::PROTECTION, 0);
Enchantment Enchantment::fireProtection(1, 5, EnchantmentCategory::ARMOR, Kind::PROTECTION, 1);
Enchantment Enchantment::fallProtection(2, 5, EnchantmentCategory::ARMOR_FEET, Kind::PROTECTION, 2);
Enchantment Enchantment::explosionProtection(3, 2, EnchantmentCategory::ARMOR, Kind::PROTECTION, 3);
Enchantment Enchantment::projectileProtection(4, 5, EnchantmentCategory::ARMOR, Kind::PROTECTION, 4);
Enchantment Enchantment::oxygen(5, 2, EnchantmentCategory::ARMOR_HEAD, Kind::OXYGEN);
Enchantment Enchantment::waterWorker(6, 2, EnchantmentCategory::ARMOR_HEAD, Kind::WATER_WORKER);
Enchantment Enchantment::damageAll(16, 10, EnchantmentCategory::WEAPON, Kind::DAMAGE, 0);
Enchantment Enchantment::damageUndead(17, 5, EnchantmentCategory::WEAPON, Kind::DAMAGE, 1);
Enchantment Enchantment::damageArthropods(18, 5, EnchantmentCategory::WEAPON, Kind::DAMAGE, 2);
Enchantment Enchantment::knockback(19, 5, EnchantmentCategory::WEAPON, Kind::KNOCKBACK);
Enchantment Enchantment::fireAspect(20, 2, EnchantmentCategory::WEAPON, Kind::FIRE);
Enchantment Enchantment::lootBonus(21, 2, EnchantmentCategory::WEAPON, Kind::LOOT);
Enchantment Enchantment::digging(32, 10, EnchantmentCategory::DIGGER, Kind::DIGGING);
Enchantment Enchantment::untouching(33, 1, EnchantmentCategory::DIGGER, Kind::UNTOUCHING);
Enchantment Enchantment::durability(34, 5, EnchantmentCategory::DIGGER, Kind::DURABILITY);
Enchantment Enchantment::lootBonusDigger(35, 2, EnchantmentCategory::DIGGER, Kind::LOOT);

namespace
{
	const int_t protMin[] = {1, 10, 5, 5, 3};
	const int_t protStep[] = {16, 8, 6, 8, 6};
	const int_t protRange[] = {20, 12, 10, 12, 15};
	const char16_t *const protNames[] = {u"all", u"fire", u"fall", u"explosion", u"projectile"};
	const int_t dmgMin[] = {1, 5, 5};
	const int_t dmgStep[] = {16, 8, 8};
	const char16_t *const dmgNames[] = {u"all", u"undead", u"arthropods"};

	int_t baseMinCost(int_t level)
	{
		return 1 + level * 10;
	}
}

bool canEnchant(EnchantmentCategory category, const Item &item)
{
	if (category == EnchantmentCategory::ALL)
		return true;
	if (const ItemArmor *armor = dynamic_cast<const ItemArmor *>(&item))
	{
		if (category == EnchantmentCategory::ARMOR)
			return true;
		switch (armor->armorType)
		{
		case 0: return category == EnchantmentCategory::ARMOR_HEAD;
		case 1: return category == EnchantmentCategory::ARMOR_TORSO;
		case 2: return category == EnchantmentCategory::ARMOR_LEGS;
		case 3: return category == EnchantmentCategory::ARMOR_FEET;
		default: return false;
		}
	}
	if (dynamic_cast<const ItemSword *>(&item) != nullptr)
		return category == EnchantmentCategory::WEAPON;
	if (dynamic_cast<const ItemTool *>(&item) != nullptr)
		return category == EnchantmentCategory::DIGGER;
	return false;
}

Enchantment::Enchantment(int_t id, int_t weight, EnchantmentCategory category, Kind kind, int_t subType)
	: id(id), weight(weight), category(category), kind(kind), subType(subType)
{
	enchantments[id] = this;
}

int_t Enchantment::getMaxLevel() const
{
	switch (kind)
	{
	case Kind::PROTECTION: return 4;
	case Kind::DAMAGE: return 5;
	case Kind::OXYGEN: return 3;
	case Kind::WATER_WORKER: return 1;
	case Kind::KNOCKBACK: return 2;
	case Kind::FIRE: return 2;
	case Kind::LOOT: return 3;
	case Kind::DIGGING: return 5;
	case Kind::UNTOUCHING: return 1;
	case Kind::DURABILITY: return 3;
	}
	return 1;
}

int_t Enchantment::getMinCost(int_t level) const
{
	switch (kind)
	{
	case Kind::PROTECTION: return protMin[subType] + (level - 1) * protStep[subType];
	case Kind::DAMAGE: return dmgMin[subType] + (level - 1) * dmgStep[subType];
	case Kind::OXYGEN: return 10 * level;
	case Kind::WATER_WORKER: return 1;
	case Kind::KNOCKBACK: return 5 + 20 * (level - 1);
	case Kind::FIRE: return 10 + 20 * (level - 1);
	case Kind::LOOT: return 20 + (level - 1) * 12;
	case Kind::DIGGING: return 1 + 15 * (level - 1);
	case Kind::UNTOUCHING: return 25;
	case Kind::DURABILITY: return 5 + (level - 1) * 10;
	}
	return baseMinCost(level);
}

int_t Enchantment::getMaxCost(int_t level) const
{
	switch (kind)
	{
	case Kind::PROTECTION: return getMinCost(level) + protRange[subType];
	case Kind::DAMAGE: return getMinCost(level) + 20;
	case Kind::OXYGEN: return getMinCost(level) + 30;
	case Kind::WATER_WORKER: return getMinCost(level) + 40;
	default:
		// 1.0 uses super.getMinXpRequirement here, the base 1 + level * 10
		return baseMinCost(level) + 50;
	}
}

int_t Enchantment::getDamageProtection(int_t level, DamageKind damage, bool projectile) const
{
	if (kind != Kind::PROTECTION || damage == DamageKind::OUT_OF_WORLD)
		return 0;
	int_t value = (6 + level * level) / 2;
	switch (subType)
	{
	case 0: return value;
	case 1: return damage == DamageKind::FIRE ? value : 0;
	case 2: return damage == DamageKind::FALL ? value * 2 : 0;
	case 3: return damage == DamageKind::EXPLOSION ? value : 0;
	case 4: return projectile ? value : 0;
	}
	return 0;
}

int_t Enchantment::getDamageBonus(int_t level, Mob &target) const
{
	if (kind != Kind::DAMAGE)
		return 0;
	if (subType == 0)
		return level * 3;
	if (subType == 1 && target.isUndead())
		return level * 4;
	if (subType == 2 && target.isArthropod())
		return level * 4;
	return 0;
}

bool Enchantment::isCompatibleWith(const Enchantment &other) const
{
	if (kind == Kind::PROTECTION && other.kind == Kind::PROTECTION)
		return other.subType == subType ? false : (subType == 2 || other.subType == 2);
	if (kind == Kind::DAMAGE)
		return other.kind != Kind::DAMAGE;
	if (kind == Kind::UNTOUCHING)
		return this != &other && &other != &lootBonusDigger;
	if (kind == Kind::LOOT)
		return this != &other && &other != &untouching;
	return this != &other;
}

jstring Enchantment::getDescriptionId() const
{
	switch (kind)
	{
	case Kind::PROTECTION: return jstring(u"enchantment.protect.") + protNames[subType];
	case Kind::DAMAGE: return jstring(u"enchantment.damage.") + dmgNames[subType];
	case Kind::OXYGEN: return u"enchantment.oxygen";
	case Kind::WATER_WORKER: return u"enchantment.waterWorker";
	case Kind::KNOCKBACK: return u"enchantment.knockback";
	case Kind::FIRE: return u"enchantment.fire";
	case Kind::LOOT: return category == EnchantmentCategory::DIGGER ? u"enchantment.lootBonusDigger" : u"enchantment.lootBonus";
	case Kind::DIGGING: return u"enchantment.digging";
	case Kind::UNTOUCHING: return u"enchantment.untouching";
	case Kind::DURABILITY: return u"enchantment.durability";
	}
	return u"enchantment.unknown";
}

jstring Enchantment::getFullname(int_t level) const
{
	return StatCollector::translate(getDescriptionId()) + u" " + StatCollector::translate(u"enchantment.level." + String::toString(level));
}

int_t EnchantmentHelper::getEnchantmentLevel(int_t id, const ItemInstance *item)
{
	if (item == nullptr || item->isEmpty())
		return 0;
	for (const auto &e : item->enchantments)
		if (e.first == id)
			return e.second;
	return 0;
}

int_t EnchantmentHelper::getHighestLevel(int_t id, InventoryPlayer &inventory)
{
	int_t best = 0;
	for (const ItemInstance &armor : inventory.armorInventory)
	{
		int_t level = getEnchantmentLevel(id, &armor);
		if (level > best)
			best = level;
	}
	return best;
}

int_t EnchantmentHelper::getDamageProtection(InventoryPlayer &inventory, DamageKind kind, bool projectile)
{
	static Random random;
	int_t total = 0;
	for (const ItemInstance &armor : inventory.armorInventory)
	{
		if (armor.isEmpty())
			continue;
		for (const auto &e : armor.enchantments)
			if (Enchantment::enchantments[e.first] != nullptr)
				total += Enchantment::enchantments[e.first]->getDamageProtection(e.second, kind, projectile);
	}
	if (total > 25)
		total = 25;
	return (total + 1 >> 1) + random.nextInt((total >> 1) + 1);
}

int_t EnchantmentHelper::getDamageBonus(InventoryPlayer &inventory, Mob &target)
{
	static Random random;
	const ItemInstance *held = inventory.getCurrentItem();
	int_t total = 0;
	if (held != nullptr)
		for (const auto &e : held->enchantments)
			if (Enchantment::enchantments[e.first] != nullptr)
				total += Enchantment::enchantments[e.first]->getDamageBonus(e.second, target);
	return total > 0 ? 1 + random.nextInt(total) : 0;
}

int_t EnchantmentHelper::getKnockbackBonus(InventoryPlayer &inventory) { return getEnchantmentLevel(Enchantment::knockback.id, inventory.getCurrentItem()); }
int_t EnchantmentHelper::getFireAspect(InventoryPlayer &inventory) { return getEnchantmentLevel(Enchantment::fireAspect.id, inventory.getCurrentItem()); }
int_t EnchantmentHelper::getOxygenBonus(InventoryPlayer &inventory) { return getHighestLevel(Enchantment::oxygen.id, inventory); }
int_t EnchantmentHelper::getDiggingBonus(InventoryPlayer &inventory) { return getEnchantmentLevel(Enchantment::digging.id, inventory.getCurrentItem()); }
int_t EnchantmentHelper::getDigDurability(InventoryPlayer &inventory) { return getEnchantmentLevel(Enchantment::durability.id, inventory.getCurrentItem()); }
bool EnchantmentHelper::hasSilkTouch(InventoryPlayer &inventory) { return getEnchantmentLevel(Enchantment::untouching.id, inventory.getCurrentItem()) > 0; }
int_t EnchantmentHelper::getDiggingLootBonus(InventoryPlayer &inventory) { return getEnchantmentLevel(Enchantment::lootBonusDigger.id, inventory.getCurrentItem()); }
int_t EnchantmentHelper::getKillingLootBonus(InventoryPlayer &inventory) { return getEnchantmentLevel(Enchantment::lootBonus.id, inventory.getCurrentItem()); }
bool EnchantmentHelper::hasWaterWorkerBonus(InventoryPlayer &inventory) { return getHighestLevel(Enchantment::waterWorker.id, inventory) > 0; }

int_t EnchantmentHelper::getEnchantmentCost(Random &random, int_t slot, int_t bookcases, const ItemInstance &item)
{
	Item *type = item.getItem();
	if (type == nullptr || type->getEnchantmentValue() <= 0)
		return 0;
	if (bookcases > 30)
		bookcases = 30;
	bookcases = 1 + random.nextInt((bookcases >> 1) + 1) + random.nextInt(bookcases + 1);
	int_t cost = random.nextInt(5) + bookcases;
	if (slot == 0)
		return (cost >> 1) + 1;
	return slot == 1 ? cost * 2 / 3 + 1 : cost;
}

std::map<int_t, EnchantmentHelper::Entry> EnchantmentHelper::getAvailableEnchantmentResults(int_t value, const ItemInstance &item)
{
	std::map<int_t, Entry> results;
	Item *type = item.getItem();
	if (type == nullptr)
		return results;
	for (Enchantment *e : Enchantment::enchantments)
	{
		if (e == nullptr || !canEnchant(e->category, *type))
			continue;
		for (int_t level = e->getMinLevel(); level <= e->getMaxLevel(); ++level)
			if (value >= e->getMinCost(level) && value <= e->getMaxCost(level))
				results[e->id] = Entry{e, level};
	}
	return results;
}

namespace
{
	const EnchantmentHelper::Entry *pick(Random &random, const std::map<int_t, EnchantmentHelper::Entry> &entries)
	{
		int_t total = 0;
		for (const auto &kv : entries)
			total += kv.second.enchantment->weight;
		if (total <= 0)
			return nullptr;
		int_t r = random.nextInt(total);
		for (const auto &kv : entries)
		{
			r -= kv.second.enchantment->weight;
			if (r < 0)
				return &kv.second;
		}
		return nullptr;
	}
}

std::vector<EnchantmentHelper::Entry> EnchantmentHelper::selectEnchantment(Random &random, const ItemInstance &item, int_t cost)
{
	std::vector<Entry> result;
	Item *type = item.getItem();
	if (type == nullptr)
		return result;
	int_t value = type->getEnchantmentValue();
	if (value <= 0)
		return result;
	value = 1 + random.nextInt((value >> 1) + 1) + random.nextInt((value >> 1) + 1);
	int_t total = value + cost;
	float spread = (random.nextFloat() + random.nextFloat() - 1.0f) * 0.25f;
	int_t finalValue = static_cast<int_t>(total * (1.0f + spread) + 0.5f);
	std::map<int_t, Entry> available = getAvailableEnchantmentResults(finalValue, item);
	if (available.empty())
		return result;
	const Entry *first = pick(random, available);
	if (first == nullptr)
		return result;
	result.push_back(*first);
	for (int_t chance = finalValue >> 1; random.nextInt(50) <= chance; chance >>= 1)
	{
		for (auto it = available.begin(); it != available.end();)
		{
			bool ok = true;
			for (const Entry &chosen : result)
				if (!chosen.enchantment->isCompatibleWith(*Enchantment::enchantments[it->first]))
				{
					ok = false;
					break;
				}
			it = ok ? std::next(it) : available.erase(it);
		}
		if (!available.empty())
		{
			const Entry *next = pick(random, available);
			if (next != nullptr)
				result.push_back(*next);
		}
	}
	return result;
}
