#include "world/entity/ai/RandomPos.h"

#include "world/entity/PathfinderMob.h"
#include "util/Mth.h"

namespace RandomPos
{

static bool generateRandomPos(PathfinderMob &mob, int_t xzDist, int_t yDist, const Vec3 *dir, Vec3 &out)
{
	Random &random = mob.getRandom();
	bool hasBest = false;
	int_t xBest = 0, yBest = 0, zBest = 0;
	float best = -99999.0f;
	bool restrict;
	if (mob.hasRestriction())
	{
		double dist = mob.getRestrictCenterDistanceSqr(Mth::floor(mob.x), Mth::floor(mob.y), Mth::floor(mob.z)) + 4.0f;
		double radius = mob.getRestrictRadius() + xzDist;
		restrict = dist < radius * radius;
	}
	else
		restrict = false;

	for (int_t i = 0; i < 10; ++i)
	{
		int_t xt = random.nextInt(2 * xzDist) - xzDist;
		int_t yt = random.nextInt(2 * yDist) - yDist;
		int_t zt = random.nextInt(2 * xzDist) - xzDist;
		if (dir != nullptr && xt * dir->x + zt * dir->z < 0.0)
			continue;
		xt += Mth::floor(mob.x);
		yt += Mth::floor(mob.y);
		zt += Mth::floor(mob.z);
		if (restrict && !mob.isWithinRestriction(xt, yt, zt))
			continue;
		float value = mob.getWalkTargetValue(xt, yt, zt);
		if (value > best)
		{
			best = value;
			xBest = xt;
			yBest = yt;
			zBest = zt;
			hasBest = true;
		}
	}
	if (!hasBest)
		return false;
	out = Vec3(xBest, yBest, zBest);
	return true;
}

bool getPos(PathfinderMob &mob, int_t xzDist, int_t yDist, Vec3 &out)
{
	return generateRandomPos(mob, xzDist, yDist, nullptr, out);
}

bool getPosTowards(PathfinderMob &mob, int_t xzDist, int_t yDist, const Vec3 &towards, Vec3 &out)
{
	Vec3 dir(towards.x - mob.x, towards.y - mob.y, towards.z - mob.z);
	return generateRandomPos(mob, xzDist, yDist, &dir, out);
}

bool getPosAvoid(PathfinderMob &mob, int_t xzDist, int_t yDist, const Vec3 &avoid, Vec3 &out)
{
	Vec3 dir(mob.x - avoid.x, mob.y - avoid.y, mob.z - avoid.z);
	return generateRandomPos(mob, xzDist, yDist, &dir, out);
}

}
