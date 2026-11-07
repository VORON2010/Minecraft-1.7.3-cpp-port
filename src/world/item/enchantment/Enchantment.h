#pragma once

#include <array>
#include <map>
#include <utility>
#include <vector>

#include "java/Random.h"
#include "java/String.h"
#include "java/Type.h"

class Item;
class ItemInstance;
class InventoryPlayer;
class Mob;

// 1.0 EnchantmentCategory
enum class EnchantmentCategory
{
	ALL,
	ARMOR,
	ARMOR_FEET,
	ARMOR_LEGS,
	ARMOR_TORSO,
	ARMOR_HEAD,
	WEAPON,
	DIGGER,
};

bool canEnchant(EnchantmentCategory category, const Item &item);

// what kind of hit is being taken, for the protection enchantments (1.0 DamageSource)
enum class DamageKind
{
	GENERIC,
	FIRE,
	FALL,
	EXPLOSION,
	OUT_OF_WORLD,
};

// 1.0 Enchantment and its subclasses
class Enchantment
{
public:
	static std::array<Enchantment *, 256> enchantments;

	static Enchantment protection;
	static Enchantment fireProtection;
	static Enchantment fallProtection;
	static Enchantment explosionProtection;
	static Enchantment projectileProtection;
	static Enchantment oxygen;
	static Enchantment waterWorker;
	static Enchantment damageAll;
	static Enchantment damageUndead;
	static Enchantment damageArthropods;
	static Enchantment knockback;
	static Enchantment fireAspect;
	static Enchantment lootBonus;
	static Enchantment digging;
	static Enchantment untouching;
	static Enchantment durability;
	static Enchantment lootBonusDigger;

	enum class Kind
	{
		PROTECTION,
		DAMAGE,
		OXYGEN,
		WATER_WORKER,
		KNOCKBACK,
		FIRE,
		LOOT,
		DIGGING,
		UNTOUCHING,
		DURABILITY,
	};

	const int_t id;
	const int_t weight;
	EnchantmentCategory category;
	const Kind kind;
	// protection type (all, fire, fall, explosion, projectile) or damage target (all, undead, arthropods)
	const int_t subType;

	Enchantment(int_t id, int_t weight, EnchantmentCategory category, Kind kind, int_t subType = 0);
	Enchantment(const Enchantment &) = delete;

	int_t getMinLevel() const { return 1; }
	int_t getMaxLevel() const;
	int_t getMinCost(int_t level) const;
	int_t getMaxCost(int_t level) const;
	int_t getDamageProtection(int_t level, DamageKind kind, bool projectile) const;
	int_t getDamageBonus(int_t level, Mob &target) const;
	bool isCompatibleWith(const Enchantment &other) const;
	jstring getDescriptionId() const;
	jstring getFullname(int_t level) const;
};

// 1.0 EnchantmentHelper
class EnchantmentHelper
{
public:
	struct Entry
	{
		const Enchantment *enchantment;
		int_t level;
	};

	static int_t getEnchantmentLevel(int_t id, const ItemInstance *item);
	static int_t getDamageProtection(InventoryPlayer &inventory, DamageKind kind, bool projectile);
	static int_t getDamageBonus(InventoryPlayer &inventory, Mob &target);
	static int_t getKnockbackBonus(InventoryPlayer &inventory);
	static int_t getFireAspect(InventoryPlayer &inventory);
	static int_t getOxygenBonus(InventoryPlayer &inventory);
	static int_t getDiggingBonus(InventoryPlayer &inventory);
	static int_t getDigDurability(InventoryPlayer &inventory);
	static bool hasSilkTouch(InventoryPlayer &inventory);
	static int_t getDiggingLootBonus(InventoryPlayer &inventory);
	static int_t getKillingLootBonus(InventoryPlayer &inventory);
	static bool hasWaterWorkerBonus(InventoryPlayer &inventory);

	static int_t getEnchantmentCost(Random &random, int_t slot, int_t bookcases, const ItemInstance &item);
	static std::vector<Entry> selectEnchantment(Random &random, const ItemInstance &item, int_t cost);
	static std::map<int_t, Entry> getAvailableEnchantmentResults(int_t value, const ItemInstance &item);

private:
	static int_t getHighestLevel(int_t id, InventoryPlayer &inventory);
};
