#pragma once

#include "world/entity/monster/Monster.h"

class Spider : public Monster
{
public:
	Spider(Level &level);
	jstring getEncodeId() const override { return u"Spider"; }
	double getRideHeight() override;
	bool onLadder() override;
	bool canBeAffected(const MobEffectInstance &effect) override { return effect.getId() != MobEffect::poison.id && Monster::canBeAffected(effect); }

protected:
	std::shared_ptr<Entity> findAttackTarget() override;
	jstring getAmbientSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	void checkHurtTarget(Entity &entity, float distance) override;
	int_t getDeathLoot() override;
};
