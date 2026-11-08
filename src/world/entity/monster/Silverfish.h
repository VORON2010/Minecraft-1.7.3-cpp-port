#pragma once

#include "world/entity/monster/Monster.h"

// b1.8 silverfish: hides in stone, calls friends out of monster egg blocks when hurt
class Silverfish : public Monster
{
private:
	int_t lookForFriends = 0;

public:
	void playStepSound(int_t x, int_t y, int_t z, int_t tile) override;
	bool isArthropod() override { return true; }
	Silverfish(Level &level);
	jstring getEncodeId() const override { return u"Silverfish"; }
	void tick() override;
	bool hurt(Entity *source, int_t damage) override;

protected:
	std::shared_ptr<Entity> findAttackTarget() override;
	jstring getAmbientSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	void checkHurtTarget(Entity &entity, float distance) override;
	int_t getDeathLoot() override;
	void updateAi() override;
	float getWalkTargetValue(int_t x, int_t y, int_t z) override;
};
