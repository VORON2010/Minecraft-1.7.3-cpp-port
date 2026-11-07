#pragma once

#include "world/entity/Entity.h"

class Player;

// b1.8 experience orb: dropped by mobs killed by a player, flies to the nearest player
class ExperienceOrb : public Entity
{
private:
	int_t health = 5;
	int_t value = 0;

public:
	int_t tickCount2 = 0;
	int_t age = 0;
	int_t throwTime = 0;

	ExperienceOrb(Level &level);
	ExperienceOrb(Level &level, double x, double y, double z, int_t value);

	jstring getEncodeId() const override { return u"XPOrb"; }
	void tick() override;
	bool hurt(Entity *source, int_t damage) override;
	void playerTouch(Player &player) override;
	float getBrightness(float a) override;

	int_t getValue() const { return value; }
	int_t getIcon() const;
	static int_t getExperienceValue(int_t maxValue);

protected:
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
};
