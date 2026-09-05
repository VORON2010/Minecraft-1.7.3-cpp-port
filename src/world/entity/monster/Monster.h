#pragma once

#include "world/entity/PathfinderMob.h"

class Monster : public PathfinderMob
{
protected:
	int_t attackDamage = 2;

public:
	Monster(Level &level);
	jstring getEncodeId() const override { return u"Monster"; }
	void tick() override;
	void aiStep() override;
	float getHeadHeight() override;
	bool hurt(Entity *source, int_t damage) override;
	bool canSpawn() override;

protected:
	std::shared_ptr<Entity> findAttackTarget() override;
	void checkHurtTarget(Entity &entity, float distance) override;
	float getWalkTargetValue(int_t x, int_t y, int_t z) override;
};
