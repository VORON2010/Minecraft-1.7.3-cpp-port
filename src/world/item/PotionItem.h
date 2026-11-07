#pragma once

#include <map>
#include <vector>

#include "world/item/Item.h"
#include "world/entity/effect/MobEffect.h"

// 1.0 PotionItem (373): data bits decide the effects, bit 14 makes it a splash potion
class PotionItem : public Item
{
	mutable std::map<int_t, std::vector<MobEffectInstance>> cache;

public:
	explicit PotionItem(int_t baseId);

	const std::vector<MobEffectInstance> &getMobEffects(int_t data) const;
	static bool isThrowable(int_t data) { return (data & 16384) != 0; }
	bool hasInstantenousEffects(int_t data) const;

	void use(ItemInstance &stack, Level &level, Player &player) const override;
	void useTimeDepleted(ItemInstance &stack, Level &level, Player &player) const override;
	bool useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const override;
	UseAnim getUseAnimation(const ItemInstance &stack) const override { (void)stack; return UseAnim::DRINK; }
	int_t getUseDuration(const ItemInstance &stack) const override { (void)stack; return 32; }
	int_t getIcon(const ItemInstance &stack) const override;
	int_t getColor(int_t data) const override;
	jstring getHoverName(const ItemInstance &stack) const override;
	void appendHoverText(const ItemInstance &stack, std::vector<jstring> &lines) const override;
	bool isFoil(const ItemInstance &stack) const override;
};

// 1.0 GlassBottleItem (374): fills from water
class BottleItem : public Item
{
public:
	using Item::Item;
	void use(ItemInstance &stack, Level &level, Player &player) const override;
};

// 1.0 PlaceableItem: an item that places a block (cauldron, brewing stand)
class TilePlanterItem : public Item
{
	int_t tileId;

public:
	TilePlanterItem(int_t baseId, int_t tileId) : Item(baseId), tileId(tileId) {}
	bool useOn(ItemInstance &stack, Player &player, Level &level, int_t x, int_t y, int_t z, Facing face) const override;
};

// 1.4.7 BookItem: a single book can go on the enchanting table
class BookItem : public Item
{
public:
	using Item::Item;
	int_t getEnchantmentValue() const override { return 1; }
};

// 1.4.7 EnchantedBookItem (403): holds stored enchantments for the anvil
class EnchantedBookItem : public Item
{
public:
	using Item::Item;
	bool isFoil(const ItemInstance &stack) const override { (void)stack; return true; }
	static void addEnchantment(ItemInstance &stack, int_t id, int_t level);
};
