#include "world/item/alchemy/PotionBrewing.h"

#include <array>
#include <cmath>
#include <map>

const jstring PotionBrewing::SUGAR = u"-0+1-2-3&4-4+13";
const jstring PotionBrewing::GHAST_TEAR = u"+0-1-2-3&4-4+13";
const jstring PotionBrewing::SPIDER_EYE = u"-0-1+2-3&4-4+13";
const jstring PotionBrewing::FERMENTED_SPIDER_EYE = u"-0+3-4+13";
const jstring PotionBrewing::GLISTERING_MELON = u"+0-1+2-3&4-4+13";
const jstring PotionBrewing::BLAZE_POWDER = u"+0-1-2+3&4-4+13";
const jstring PotionBrewing::MAGMA_CREAM = u"+0+1-2-3&4-4+13";
const jstring PotionBrewing::REDSTONE = u"-5+6-7";
const jstring PotionBrewing::GLOWSTONE = u"+5-6-7";
const jstring PotionBrewing::GUNPOWDER = u"+14&13-13";
const jstring PotionBrewing::NETHER_WART = u"+4";

namespace
{
	const std::array<jstring, 32> appearances = {
		u"potion.prefix.mundane", u"potion.prefix.uninteresting", u"potion.prefix.bland", u"potion.prefix.clear",
		u"potion.prefix.milky", u"potion.prefix.diffuse", u"potion.prefix.artless", u"potion.prefix.thin",
		u"potion.prefix.awkward", u"potion.prefix.flat", u"potion.prefix.bulky", u"potion.prefix.bungling",
		u"potion.prefix.buttered", u"potion.prefix.smooth", u"potion.prefix.suave", u"potion.prefix.debonair",
		u"potion.prefix.thick", u"potion.prefix.elegant", u"potion.prefix.fancy", u"potion.prefix.charming",
		u"potion.prefix.dashing", u"potion.prefix.refined", u"potion.prefix.cordial", u"potion.prefix.sparkling",
		u"potion.prefix.potent", u"potion.prefix.foul", u"potion.prefix.odorless", u"potion.prefix.rank",
		u"potion.prefix.harsh", u"potion.prefix.acrid", u"potion.prefix.gross", u"potion.prefix.stinky",
	};

	const std::map<int_t, jstring> &durations()
	{
		static const std::map<int_t, jstring> m = {
			{MobEffect::regeneration.id, u"0 & !1 & !2 & !3 & 0+6"},
			{MobEffect::movementSpeed.id, u"!0 & 1 & !2 & !3 & 1+6"},
			{MobEffect::fireResistance.id, u"0 & 1 & !2 & !3 & 0+6"},
			{MobEffect::heal.id, u"0 & !1 & 2 & !3"},
			{MobEffect::poison.id, u"!0 & !1 & 2 & !3 & 2+6"},
			{MobEffect::weakness.id, u"!0 & !1 & !2 & 3 & 3+6"},
			{MobEffect::harm.id, u"!0 & !1 & 2 & 3"},
			{MobEffect::movementSlowdown.id, u"!0 & 1 & !2 & 3 & 3+6"},
			{MobEffect::damageBoost.id, u"0 & !1 & !2 & 3 & 3+6"},
		};
		return m;
	}

	const std::map<int_t, jstring> &potencies()
	{
		static const std::map<int_t, jstring> m = {
			{MobEffect::movementSpeed.id, u"5"},
			{MobEffect::digSpeed.id, u"5"},
			{MobEffect::damageBoost.id, u"5"},
			{MobEffect::regeneration.id, u"5"},
			{MobEffect::harm.id, u"5"},
			{MobEffect::heal.id, u"5"},
			{MobEffect::damageResistance.id, u"5"},
			{MobEffect::poison.id, u"5"},
		};
		return m;
	}

	std::map<int_t, int_t> colorCache;
}

int_t PotionBrewing::getAppearanceValue(int_t data)
{
	return valueOf(data, 5, 4, 3, 2, 1);
}

int_t PotionBrewing::getColorValue(const std::vector<MobEffectInstance> &effects)
{
	if (effects.empty())
		return 3694022;
	float r = 0.0f, g = 0.0f, b = 0.0f, n = 0.0f;
	for (const MobEffectInstance &e : effects)
	{
		int_t col = MobEffect::effects[e.getId()]->getColor();
		for (int_t i = 0; i <= e.getAmplifier(); ++i)
		{
			r += static_cast<float>(col >> 16 & 0xFF) / 255.0f;
			g += static_cast<float>(col >> 8 & 0xFF) / 255.0f;
			b += static_cast<float>(col & 0xFF) / 255.0f;
			n++;
		}
	}
	r = r / n * 255.0f;
	g = g / n * 255.0f;
	b = b / n * 255.0f;
	return static_cast<int_t>(r) << 16 | static_cast<int_t>(g) << 8 | static_cast<int_t>(b);
}

int_t PotionBrewing::getColorValue(int_t data, bool includeDisabled)
{
	if (includeDisabled)
		return getColorValue(getEffects(data, true));
	auto it = colorCache.find(data);
	if (it != colorCache.end())
		return it->second;
	int_t col = getColorValue(getEffects(data, false));
	colorCache[data] = col;
	return col;
}

const jstring &PotionBrewing::getAppearanceName(int_t data)
{
	return appearances[getAppearanceValue(data)];
}

int_t PotionBrewing::applyOp(bool invert, bool multiply, bool negate, int_t op, int_t bit, int_t mult, int_t data)
{
	int_t v = 0;
	if (invert)
		v = isBitSet(data, bit) ? 0 : 1;
	else if (op != -1)
	{
		if (op == 0 && countOnes(data) == bit)
			v = 1;
		else if (op == 1 && countOnes(data) > bit)
			v = 1;
		else if (op == 2 && countOnes(data) < bit)
			v = 1;
	}
	else
		v = isBitSet(data, bit) ? 1 : 0;
	if (multiply)
		v *= mult;
	if (negate)
		v *= -1;
	return v;
}

int_t PotionBrewing::countOnes(int_t data)
{
	int_t n = 0;
	for (; data > 0; n++)
		data &= data - 1;
	return n;
}

int_t PotionBrewing::parseEffectFormula(const jstring &formula, int_t from, int_t to, int_t data)
{
	if (from >= static_cast<int_t>(formula.length()) || to < 0 || from >= to)
		return 0;
	size_t bar = formula.find(u'|', from);
	if (bar != jstring::npos && static_cast<int_t>(bar) < to)
	{
		int_t a = parseEffectFormula(formula, from, static_cast<int_t>(bar) - 1, data);
		if (a > 0)
			return a;
		int_t b = parseEffectFormula(formula, static_cast<int_t>(bar) + 1, to, data);
		return b > 0 ? b : 0;
	}
	size_t amp = formula.find(u'&', from);
	if (amp != jstring::npos && static_cast<int_t>(amp) < to)
	{
		int_t a = parseEffectFormula(formula, from, static_cast<int_t>(amp) - 1, data);
		if (a <= 0)
			return 0;
		int_t b = parseEffectFormula(formula, static_cast<int_t>(amp) + 1, to, data);
		if (b <= 0)
			return 0;
		return a > b ? a : b;
	}

	bool isMult = false, hasMult = false, hasNum = false, invert = false, negate = false;
	int_t op = -1;
	int_t bit = 0, mult = 0, total = 0;
	auto flush = [&]() {
		total += applyOp(invert, hasMult, negate, op, bit, mult, data);
		invert = negate = isMult = hasMult = hasNum = false;
		mult = bit = 0;
		op = -1;
	};
	for (int_t i = from; i < to; ++i)
	{
		char_t c = formula[i];
		if (c >= u'0' && c <= u'9')
		{
			if (isMult)
			{
				mult = c - u'0';
				hasMult = true;
			}
			else
			{
				bit = bit * 10 + (c - u'0');
				hasNum = true;
			}
		}
		else if (c == u'*')
			isMult = true;
		else if (c == u'!')
		{
			if (hasNum)
				flush();
			invert = true;
		}
		else if (c == u'-')
		{
			if (hasNum)
				flush();
			negate = true;
		}
		else if (c != u'=' && c != u'<' && c != u'>')
		{
			if (c == u'+' && hasNum)
				flush();
		}
		else
		{
			if (hasNum)
				flush();
			if (c == u'=')
				op = 0;
			else if (c == u'<')
				op = 2;
			else if (c == u'>')
				op = 1;
		}
	}
	if (hasNum)
		total += applyOp(invert, hasMult, negate, op, bit, mult, data);
	return total;
}

std::vector<MobEffectInstance> PotionBrewing::getEffects(int_t data, bool includeDisabled)
{
	std::vector<MobEffectInstance> list;
	for (MobEffect *effect : MobEffect::effects)
	{
		if (effect == nullptr || (effect->isDisabled() && !includeDisabled))
			continue;
		auto d = durations().find(effect->id);
		if (d == durations().end())
			continue;
		int_t duration = parseEffectFormula(d->second, 0, static_cast<int_t>(d->second.length()), data);
		if (duration <= 0)
			continue;
		int_t amplifier = 0;
		auto p = potencies().find(effect->id);
		if (p != potencies().end())
		{
			amplifier = parseEffectFormula(p->second, 0, static_cast<int_t>(p->second.length()), data);
			if (amplifier < 0)
				amplifier = 0;
		}
		if (effect->isInstantenous())
			duration = 1;
		else
		{
			duration = 1200 * (duration * 3 + (duration - 1) * 2);
			duration >>= amplifier;
			duration = static_cast<int_t>(std::llround(duration * effect->getDurationModifier()));
			if ((data & 16384) != 0)
				duration = static_cast<int_t>(std::llround(duration * 0.75 + 0.5));
		}
		list.emplace_back(effect->id, duration, amplifier);
	}
	return list;
}

int_t PotionBrewing::applyBrewBit(int_t data, int_t bit, bool clear, bool toggle, bool require)
{
	if (require)
	{
		if (!isBitSet(data, bit))
			return 0;
	}
	else if (clear)
		data &= ~(1 << bit);
	else if (toggle)
	{
		if ((data & 1 << bit) != 0)
			data &= ~(1 << bit);
		else
			data |= 1 << bit;
	}
	else
		data |= 1 << bit;
	return data;
}

int_t PotionBrewing::applyBrew(int_t data, const jstring &formula)
{
	bool hasNum = false, toggle = false, clear = false, require = false;
	int_t bit = 0;
	auto flush = [&]() {
		data = applyBrewBit(data, bit, clear, toggle, require);
		require = toggle = clear = hasNum = false;
		bit = 0;
	};
	for (char_t c : formula)
	{
		if (c >= u'0' && c <= u'9')
		{
			bit = bit * 10 + (c - u'0');
			hasNum = true;
		}
		else if (c == u'!')
		{
			if (hasNum)
				flush();
			toggle = true;
		}
		else if (c == u'-')
		{
			if (hasNum)
				flush();
			clear = true;
		}
		else if (c == u'+')
		{
			if (hasNum)
				flush();
		}
		else if (c == u'&')
		{
			if (hasNum)
				flush();
			require = true;
		}
	}
	if (hasNum)
		data = applyBrewBit(data, bit, clear, toggle, require);
	return data & 32767;
}

int_t PotionBrewing::valueOf(int_t data, int_t b1, int_t b2, int_t b3, int_t b4, int_t b5)
{
	return (isBitSet(data, b1) ? 16 : 0) | (isBitSet(data, b2) ? 8 : 0) | (isBitSet(data, b3) ? 4 : 0) |
		   (isBitSet(data, b4) ? 2 : 0) | (isBitSet(data, b5) ? 1 : 0);
}
