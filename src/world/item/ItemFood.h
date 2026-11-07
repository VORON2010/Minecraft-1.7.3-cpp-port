#pragma once

#include "world/item/Item.h"

class ItemFood : public Item
{
private:
	int_t healAmount = 0;
	float saturationModifier = 0.6f;
	bool wolfsFavoriteMeat = false;
	bool alwaysEdible = false;

	int_t effectId = 0;
	int_t effectDuration = 0;
	int_t effectAmplifier = 0;
	float effectChance = 0.0f;

public:
	ItemFood(int_t baseId, int_t healAmount, float saturationModifier, bool wolfsFavoriteMeat);
	void use(ItemInstance &stack, Level &level, Player &player) const override;
	void useTimeDepleted(ItemInstance &stack, Level &level, Player &player) const override;
	UseAnim getUseAnimation(const ItemInstance &stack) const override { (void)stack; return UseAnim::EAT; }
	int_t getUseDuration(const ItemInstance &stack) const override { (void)stack; return 32; }
	bool isAlwaysEdible() const { return alwaysEdible; }

	// b1.8: some food applies a status effect (raw chicken, rotten flesh)
	ItemFood &setEffect(int_t id, int_t seconds, int_t amplifier, float chance);
	ItemFood &setAlwaysEdible() { alwaysEdible = true; return *this; }

	int_t getHealAmount() const { return healAmount; }
	float getSaturationModifier() const { return saturationModifier; }
	bool isWolfsFavoriteMeat() const { return wolfsFavoriteMeat; }
};
