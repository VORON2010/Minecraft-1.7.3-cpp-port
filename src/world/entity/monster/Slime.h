#pragma once

#include <memory>

#include "world/entity/Mob.h"

// SlimeEntity from 1.0; MagmaCube overrides the hooks
class Slime : public Mob
{
public:
	float targetSquish = 0.0f;
	float squish = 0.0f;
	float squishOld = 0.0f;

private:
	int_t slimeJumpDelay = 0;

public:
	Slime(Level &level);
	jstring getEncodeId() const override { return u"Slime"; }
	void setSlimeSize(int_t size);
	int_t getExperienceReward() override { return getSlimeSize(); }
	int_t getSlimeSize() const;
	void tick() override;
	void playerTouch(Player &player) override;
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

	virtual jstring getParticleName();
	virtual jstring getSquishSound();
	virtual void decreaseSquish();
	virtual int_t getJumpDelay();
	virtual std::shared_ptr<Slime> createChild();
	virtual bool isDealsDamage();
	virtual int_t getAttackDamage();
	virtual bool doPlayJumpSound();
	virtual bool doPlayLandSound();
};
