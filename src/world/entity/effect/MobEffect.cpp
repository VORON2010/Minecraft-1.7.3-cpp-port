#include "world/entity/effect/MobEffect.h"

#include "world/entity/Mob.h"
#include "world/entity/player/Player.h"

std::array<MobEffect *, 32> MobEffect::effects = {};

MobEffect MobEffect::movementSpeed(1, false, 8171462, u"potion.moveSpeed", 0, 0);
MobEffect MobEffect::movementSlowdown(2, true, 5926017, u"potion.moveSlowdown", 1, 0);
MobEffect MobEffect::digSpeed(3, false, 14270531, u"potion.digSpeed", 2, 0, false, 1.5);
MobEffect MobEffect::digSlowdown(4, true, 4866583, u"potion.digSlowDown", 3, 0);
MobEffect MobEffect::damageBoost(5, false, 9643043, u"potion.damageBoost", 4, 0);
MobEffect MobEffect::heal(6, false, 16262179, u"potion.heal", -1, 0, true);
MobEffect MobEffect::harm(7, true, 4393481, u"potion.harm", -1, 0, true);
MobEffect MobEffect::jump(8, false, 7889559, u"potion.jump", 2, 1);
MobEffect MobEffect::confusion(9, true, 5578058, u"potion.confusion", 3, 1, false, 0.25);
MobEffect MobEffect::regeneration(10, false, 13458603, u"potion.regeneration", 7, 0, false, 0.25);
MobEffect MobEffect::damageResistance(11, false, 10044730, u"potion.resistance", 6, 1);
MobEffect MobEffect::fireResistance(12, false, 14981690, u"potion.fireResistance", 7, 1);
MobEffect MobEffect::waterBreathing(13, false, 3035801, u"potion.waterBreathing", 0, 2);
MobEffect MobEffect::invisibility(14, false, 8356754, u"potion.invisibility", 0, 1);
MobEffect MobEffect::blindness(15, true, 2039587, u"potion.blindness", 5, 1, false, 0.25);
MobEffect MobEffect::nightVision(16, false, 2039713, u"potion.nightVision", 4, 1);
MobEffect MobEffect::hunger(17, true, 5797459, u"potion.hunger", 1, 1);
MobEffect MobEffect::weakness(18, true, 4738376, u"potion.weakness", 5, 0);
MobEffect MobEffect::poison(19, true, 5149489, u"potion.poison", 6, 0, false, 0.25);

MobEffect::MobEffect(int_t id, bool harmful, int_t color, const jstring &descriptionId, int_t iconX, int_t iconY, bool instant, double durationModifier)
	: id(id), descriptionId(descriptionId), harmful(harmful), instant(instant), color(color)
{
	this->durationModifier = durationModifier > 0.0 ? durationModifier : (harmful ? 0.5 : 1.0);
	if (iconX >= 0)
		icon = iconX + iconY * 8;
	effects[id] = this;
}

void MobEffect::applyEffectTick(Mob &mob, int_t amplifier) const
{
	if (id == regeneration.id)
	{
		if (mob.health < 20)
			mob.heal(1);
	}
	else if (id == poison.id)
	{
		if (mob.health > 1)
			mob.hurtByMagic(nullptr, 1);
	}
	else if (id == hunger.id)
	{
		if (Player *player = dynamic_cast<Player *>(&mob))
			player->addExhaustion(0.025f * static_cast<float>(amplifier + 1));
	}
	else if ((id == heal.id && !mob.isUndead()) || (id == harm.id && mob.isUndead()))
	{
		mob.heal(6 << amplifier);
	}
	else if ((id == harm.id && !mob.isUndead()) || (id == heal.id && mob.isUndead()))
	{
		mob.hurtByMagic(nullptr, 6 << amplifier);
	}
}

void MobEffect::applyInstantenousEffect(Mob *source, Mob &target, int_t amplifier, double scale) const
{
	int_t amount = static_cast<int_t>(scale * static_cast<double>(6 << amplifier) + 0.5);
	if ((id == heal.id && !target.isUndead()) || (id == harm.id && target.isUndead()))
		target.heal(amount);
	else if ((id == harm.id && !target.isUndead()) || (id == heal.id && target.isUndead()))
		target.hurtByMagic(source, amount);
}

bool MobEffect::isDurationEffectTick(int_t duration, int_t amplifier) const
{
	if (instant)
		return duration >= 1;
	if (id == regeneration.id || id == poison.id)
	{
		int_t interval = 25 >> amplifier;
		return interval > 0 ? duration % interval == 0 : true;
	}
	return id == hunger.id;
}

void MobEffectInstance::update(const MobEffectInstance &other)
{
	if (other.amplifier > amplifier)
	{
		amplifier = other.amplifier;
		duration = other.duration;
	}
	else if (other.amplifier == amplifier && duration < other.duration)
	{
		duration = other.duration;
	}
}

bool MobEffectInstance::tick(Mob &mob)
{
	if (duration > 0)
	{
		MobEffect *effect = MobEffect::effects[effectId];
		if (effect != nullptr && effect->isDurationEffectTick(duration, amplifier))
			applyEffect(mob);
		duration--;
	}
	return duration > 0;
}

void MobEffectInstance::applyEffect(Mob &mob) const
{
	MobEffect *effect = MobEffect::effects[effectId];
	if (duration > 0 && effect != nullptr)
		effect->applyEffectTick(mob, amplifier);
}

const jstring &MobEffectInstance::getDescriptionId() const
{
	static const jstring empty;
	MobEffect *effect = MobEffect::effects[effectId];
	return effect != nullptr ? effect->getDescriptionId() : empty;
}

jstring MobEffectInstance::formatDuration(const MobEffectInstance &instance)
{
	int_t seconds = instance.getDuration() / 20;
	int_t minutes = seconds / 60;
	seconds %= 60;
	return String::toString(minutes) + (seconds < 10 ? u":0" : u":") + String::toString(seconds);
}
