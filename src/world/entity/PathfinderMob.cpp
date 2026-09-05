#include "world/entity/PathfinderMob.h"

#include "world/level/Level.h"
#include "world/level/pathfinder/PathEntity.h"
#include "world/phys/Vec3.h"
#include "util/Mth.h"

PathfinderMob::PathfinderMob(Level &level) : Mob(level)
{
}

bool PathfinderMob::isMovementCeased()
{
	return false;
}

void PathfinderMob::updateAi()
{
	holdGround = isMovementCeased();
	float targetRange = 16.0f;
	if (attackTarget == nullptr)
	{
		attackTarget = findAttackTarget();
		if (attackTarget != nullptr)
			path = level.getPathToEntity(*this, *attackTarget, targetRange);
	}
	else if (!attackTarget->isAlive())
	{
		attackTarget = nullptr;
	}
	else
	{
		float distance = attackTarget->distanceTo(*this);
		if (canSee(*attackTarget))
			checkHurtTarget(*attackTarget, distance);
		else
			attackBlockedEntity(*attackTarget, distance);
	}

	if (holdGround || attackTarget == nullptr || (path != nullptr && random.nextInt(20) != 0))
	{
		if (!holdGround && ((path == nullptr && random.nextInt(80) == 0) || random.nextInt(80) == 0))
			findRandomStrollLocation();
	}
	else
	{
		path = level.getPathToEntity(*this, *attackTarget, targetRange);
	}

	int_t currentY = Mth::floor(bb.y0 + 0.5);
	bool inWater = isInWater();
	bool inLava = isInLava();
	xRot = 0.0f;
	if (path != nullptr && random.nextInt(100) != 0)
	{
		Vec3 *nextPos = path->getPosition(*this);
		double width = bbWidth * 2.0f;
		while (nextPos != nullptr && nextPos->distanceToSqr(x, nextPos->y, z) < width * width)
		{
			path->incrementPathIndex();
			if (path->isFinished())
			{
				nextPos = nullptr;
				path.reset();
			}
			else
			{
				nextPos = path->getPosition(*this);
			}
		}

		jumping = false;
		if (nextPos != nullptr)
		{
			double dx = nextPos->x - x;
			double dz = nextPos->z - z;
			double dy = nextPos->y - currentY;
			float targetYRot = static_cast<float>(std::atan2(dz, dx) * 180.0 / Mth::PI) - 90.0f;
			float yRotDelta = targetYRot - yRot;
			yya = runSpeed;
			while (yRotDelta < -180.0f) yRotDelta += 360.0f;
			while (yRotDelta >= 180.0f) yRotDelta -= 360.0f;
			if (yRotDelta > 30.0f) yRotDelta = 30.0f;
			if (yRotDelta < -30.0f) yRotDelta = -30.0f;
			yRot += yRotDelta;
			if (holdGround && attackTarget != nullptr)
			{
				double targetDx = attackTarget->x - x;
				double targetDz = attackTarget->z - z;
				float oldYRot = yRot;
				yRot = static_cast<float>(std::atan2(targetDz, targetDx) * 180.0 / Mth::PI) - 90.0f;
				float angle = (oldYRot - yRot + 90.0f) * Mth::DEGRAD;
				xxa = -Mth::sin(angle) * yya;
				yya = Mth::cos(angle) * yya;
			}
			if (dy > 0.0)
				jumping = true;
		}

		if (attackTarget != nullptr)
			lookAt(*attackTarget, 30.0f, 30.0f);
		if (horizontalCollision && !hasPath())
			jumping = true;
		if (random.nextFloat() < 0.8f && (inWater || inLava))
			jumping = true;
	}
	else
	{
		Mob::updateAi();
		path.reset();
	}
}

void PathfinderMob::findRandomStrollLocation()
{
	bool found = false;
	int_t bestX = -1;
	int_t bestY = -1;
	int_t bestZ = -1;
	float bestWeight = -99999.0f;
	for (int_t i = 0; i < 10; ++i)
	{
		int_t targetX = Mth::floor(x + random.nextInt(13) - 6.0);
		int_t targetY = Mth::floor(y + random.nextInt(7) - 3.0);
		int_t targetZ = Mth::floor(z + random.nextInt(13) - 6.0);
		float weight = getWalkTargetValue(targetX, targetY, targetZ);
		if (weight > bestWeight)
		{
			bestWeight = weight;
			bestX = targetX;
			bestY = targetY;
			bestZ = targetZ;
			found = true;
		}
	}
	if (found)
		path = level.getEntityPathToXYZ(*this, bestX, bestY, bestZ, 10.0f);
}

void PathfinderMob::checkHurtTarget(Entity &entity, float distance)
{
	(void)entity;
	(void)distance;
}

void PathfinderMob::attackBlockedEntity(Entity &entity, float distance)
{
	(void)entity;
	(void)distance;
}

float PathfinderMob::getWalkTargetValue(int_t x, int_t y, int_t z)
{
	(void)x;
	(void)y;
	(void)z;
	return 0.0f;
}

std::shared_ptr<Entity> PathfinderMob::findAttackTarget()
{
	return nullptr;
}

bool PathfinderMob::canSpawn()
{
	int_t x = Mth::floor(this->x);
	int_t y = Mth::floor(bb.y0);
	int_t z = Mth::floor(this->z);
	return Mob::canSpawn() && getWalkTargetValue(x, y, z) >= 0.0f;
}

bool PathfinderMob::hasPath() const
{
	return path != nullptr;
}

void PathfinderMob::setPath(std::unique_ptr<PathEntity> newPath)
{
	path = std::move(newPath);
}

std::shared_ptr<Entity> PathfinderMob::getTarget() const
{
	return attackTarget;
}

void PathfinderMob::setTarget(const std::shared_ptr<Entity> &target)
{
	attackTarget = target;
}
