#pragma once

#include <vector>

#include "java/String.h"
#include "java/Type.h"
#include "world/entity/effect/MobEffect.h"

// 1.0 PotionHelper: potion data is 15 bits, ingredients flip bits, recipes read them back as effects
class PotionBrewing
{
public:
	static const jstring SUGAR;
	static const jstring GHAST_TEAR;
	static const jstring SPIDER_EYE;
	static const jstring FERMENTED_SPIDER_EYE;
	static const jstring GLISTERING_MELON;
	static const jstring BLAZE_POWDER;
	static const jstring MAGMA_CREAM;
	static const jstring REDSTONE;
	static const jstring GLOWSTONE;
	static const jstring GUNPOWDER;
	static const jstring NETHER_WART;

	static bool isBitSet(int_t data, int_t bit) { return (data & 1 << bit) != 0; }
	static int_t getAppearanceValue(int_t data);
	static int_t getColorValue(const std::vector<MobEffectInstance> &effects);
	static int_t getColorValue(int_t data, bool includeDisabled);
	static const jstring &getAppearanceName(int_t data);
	static std::vector<MobEffectInstance> getEffects(int_t data, bool includeDisabled);
	static int_t applyBrew(int_t data, const jstring &formula);
	static int_t valueOf(int_t data, int_t b1, int_t b2, int_t b3, int_t b4, int_t b5);

private:
	static int_t applyOp(bool invert, bool multiply, bool negate, int_t op, int_t bit, int_t mult, int_t data);
	static int_t countOnes(int_t data);
	static int_t parseEffectFormula(const jstring &formula, int_t from, int_t to, int_t data);
	static int_t applyBrewBit(int_t data, int_t bit, bool clear, bool toggle, bool require);
};
