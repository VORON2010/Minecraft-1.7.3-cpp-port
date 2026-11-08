#pragma once

#include "java/Type.h"
#include "world/phys/Vec3.h"

class PathfinderMob;

// 1.5.2 TargetFinder: picks the best of ten random spots near the mob,
// staying inside its home area when it has one
namespace RandomPos
{
	bool getPos(PathfinderMob &mob, int_t xzDist, int_t yDist, Vec3 &out);
	bool getPosTowards(PathfinderMob &mob, int_t xzDist, int_t yDist, const Vec3 &towards, Vec3 &out);
	bool getPosAvoid(PathfinderMob &mob, int_t xzDist, int_t yDist, const Vec3 &avoid, Vec3 &out);
}
