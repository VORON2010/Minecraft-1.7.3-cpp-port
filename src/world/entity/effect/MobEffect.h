#pragma once

#include <array>

#include "java/String.h"
#include "java/Type.h"

class Mob;

// Status effects (b1.8 / 1.0). Ids and colours match vanilla.
class MobEffect
{
public:
	static std::array<MobEffect *, 32> effects;

	static MobEffect movementSpeed;
	static MobEffect movementSlowdown;
	static MobEffect digSpeed;
	static MobEffect digSlowdown;
	static MobEffect damageBoost;
	static MobEffect heal;
	static MobEffect harm;
	static MobEffect jump;
	static MobEffect confusion;
	static MobEffect regeneration;
	static MobEffect damageResistance;
	static MobEffect fireResistance;
	static MobEffect waterBreathing;
	static MobEffect invisibility;
	static MobEffect blindness;
	static MobEffect nightVision;
	static MobEffect hunger;
	static MobEffect weakness;
	static MobEffect poison;

	const int_t id;

private:
	jstring descriptionId;
	int_t icon = -1;
	bool harmful = false;
	bool instant = false;
	bool disabled = false;
	double durationModifier = 1.0;
	int_t color = 0;

public:
	MobEffect(int_t id, bool harmful, int_t color, const jstring &descriptionId, int_t iconX, int_t iconY, bool instant = false, double durationModifier = -1.0);
	MobEffect(const MobEffect &) = delete;
	MobEffect &operator=(const MobEffect &) = delete;

	void applyEffectTick(Mob &mob, int_t amplifier) const;
	void applyInstantenousEffect(Mob *source, Mob &target, int_t amplifier, double scale) const;
	bool isDurationEffectTick(int_t duration, int_t amplifier) const;

	bool isInstantenous() const { return instant; }
	// 1.0 setUsable(): invisibility and night vision exist but no potion makes them
	MobEffect &setDisabled() { disabled = true; return *this; }
	bool isDisabled() const { return disabled; }
	bool hasIcon() const { return icon >= 0; }
	int_t getIcon() const { return icon; }
	bool isHarmful() const { return harmful; }
	double getDurationModifier() const { return durationModifier; }
	int_t getColor() const { return color; }
	const jstring &getDescriptionId() const { return descriptionId; }
};

class MobEffectInstance
{
private:
	int_t effectId = 0;
	int_t duration = 0;
	int_t amplifier = 0;

public:
	MobEffectInstance() = default;
	MobEffectInstance(int_t id, int_t duration, int_t amplifier) : effectId(id), duration(duration), amplifier(amplifier) {}

	void update(const MobEffectInstance &other);
	bool tick(Mob &mob);
	void applyEffect(Mob &mob) const;

	int_t getId() const { return effectId; }
	int_t getDuration() const { return duration; }
	int_t getAmplifier() const { return amplifier; }
	const jstring &getDescriptionId() const;

	static jstring formatDuration(const MobEffectInstance &instance);
};
