#pragma once

#include "world/entity/monster/Slime.h"

// 1.0 magma cube
class LavaSlime : public Slime
{
	int_t armorCarry = 0;

public:
	LavaSlime(Level &level);
	jstring getEncodeId() const override { return u"LavaSlime"; }
	bool canSpawn() override;
	float getBrightness(float a) override;
	bool isOnFire() override;
	bool isInLava() override;

protected:
	jstring getParticleName() override;
	std::shared_ptr<Slime> createChild() override;
	int_t getDeathLoot() override;
	void actuallyHurt(int_t dmg) override;
	int_t getJumpDelay() override;
	void decreaseSquish() override;
	void jumpFromGround() override;
	void causeFallDamage(float distance) override;
	bool isDealsDamage() override;
	int_t getAttackDamage() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	jstring getSquishSound() override;
	bool doPlayLandSound() override;
};
