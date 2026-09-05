#pragma once

#include "world/entity/Mob.h"

class Slime : public Mob
{
public:
	float squish = 0.0f;
	float squishOld = 0.0f;

private:
	int_t slimeJumpDelay = 0;

public:
	Slime(Level &level);
	jstring getEncodeId() const override { return u"Slime"; }
	void setSlimeSize(int_t size);
	int_t getSlimeSize() const;
	void tick() override;
	void playerTouch(Player &player) override;
	void die(Entity *source) override;
	bool canSpawn() override;

protected:
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
	void updateAi() override;
	void beforeRemove() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDeathLoot() override;
	float getSoundVolume() override;
};
