#pragma once

#include <memory>
#include <vector>

#include "world/item/ItemInstance.h"

class CompoundTag;
class Player;

// 1.5.2 TradeOffer (Mojang MerchantRecipe): buy A (+ optional B) -> sell, 7 uses at first
class MerchantRecipe
{
	ItemInstance buyA;
	bool hasBuyB = false;
	ItemInstance buyB;
	ItemInstance sell;
	int_t uses = 0;
	int_t maxUses = 7;

public:
	explicit MerchantRecipe(CompoundTag &tag) { load(tag); }
	MerchantRecipe(const ItemInstance &buyA, const ItemInstance &buyB, const ItemInstance &sell)
		: buyA(buyA), hasBuyB(true), buyB(buyB), sell(sell) {}
	MerchantRecipe(const ItemInstance &buy, const ItemInstance &sell) : buyA(buy), sell(sell) {}

	const ItemInstance &getBuyA() const { return buyA; }
	ItemInstance &getBuyA() { return buyA; }
	const ItemInstance *getBuyB() const { return hasBuyB ? &buyB : nullptr; }
	bool hasSecondCostItem() const { return hasBuyB; }
	const ItemInstance &getSellItem() const { return sell; }
	bool isSame(const MerchantRecipe &other) const;
	bool isSameSameButBetter(const MerchantRecipe &other) const;
	int_t getUses() const { return uses; }
	int_t getMaxUses() const { return maxUses; }
	void increaseUses() { uses++; }
	void increaseMaxUses(int_t amount) { maxUses += amount; }
	bool isDeprecated() const { return uses >= maxUses; }
	void setToOutOfStock() { uses = maxUses; }
	void load(CompoundTag &tag);
	std::shared_ptr<CompoundTag> createTag() const;
};

// 1.5.2 TradeOffers (Mojang MerchantRecipeList)
class MerchantRecipeList
{
public:
	std::vector<std::shared_ptr<MerchantRecipe>> recipes;

	MerchantRecipeList() {}
	explicit MerchantRecipeList(CompoundTag &tag) { load(tag); }

	size_t size() const { return recipes.size(); }
	bool empty() const { return recipes.empty(); }
	MerchantRecipe &operator[](size_t i) { return *recipes[i]; }
	void add(std::shared_ptr<MerchantRecipe> recipe) { recipes.push_back(std::move(recipe)); }
	MerchantRecipe *getRecipeFor(const ItemInstance &buyA, const ItemInstance *buyB, int_t selectionHint);
	void addIfNewOrBetter(std::shared_ptr<MerchantRecipe> recipe);
	void load(CompoundTag &tag);
	std::shared_ptr<CompoundTag> createTag() const;
};

// 1.5.2 Trader (Mojang Merchant)
class Merchant
{
public:
	virtual ~Merchant() {}
	virtual void setTradingPlayer(Player *player) = 0;
	virtual Player *getTradingPlayer() = 0;
	virtual MerchantRecipeList *getOffers(Player &player) = 0;
	virtual void notifyTrade(MerchantRecipe &recipe) = 0;
};
