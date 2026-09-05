#pragma once

#include <memory>

#include "world/entity/Mob.h"
#include "world/level/pathfinder/PathEntity.h"


class PathfinderMob : public Mob
{
protected:
	std::unique_ptr<PathEntity> path;
	std::shared_ptr<Entity> attackTarget;
	bool holdGround = false;

public:
	PathfinderMob(Level &level);

protected:
	virtual bool isMovementCeased();
	void updateAi() override;
	virtual void findRandomStrollLocation();
	virtual void checkHurtTarget(Entity &entity, float distance);
	virtual void attackBlockedEntity(Entity &entity, float distance);
	virtual float getWalkTargetValue(int_t x, int_t y, int_t z);
	virtual std::shared_ptr<Entity> findAttackTarget();

public:
	bool canSpawn() override;
	bool hasPath() const;
	void setPath(std::unique_ptr<PathEntity> newPath);
	std::shared_ptr<Entity> getTarget() const;
	void setTarget(const std::shared_ptr<Entity> &target);
};
