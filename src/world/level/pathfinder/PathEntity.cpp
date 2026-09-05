#include "world/level/pathfinder/PathEntity.h"

#include <utility>

#include "world/entity/Entity.h"
#include "world/phys/Vec3.h"
#include "util/Mth.h"

PathEntity::PathEntity(std::vector<PathPoint> &&points) : points(std::move(points)), pathLength(static_cast<int_t>(this->points.size()))
{
}

void PathEntity::incrementPathIndex()
{
	pathIndex++;
}

bool PathEntity::isFinished() const
{
	return pathIndex >= static_cast<int_t>(points.size());
}

const PathPoint *PathEntity::getEndPoint() const
{
	return points.empty() ? nullptr : &points.back();
}

Vec3 *PathEntity::getPosition(Entity &entity) const
{
	if (isFinished())
		return nullptr;
	const PathPoint &point = points[pathIndex];
	double x = point.xCoord + static_cast<int_t>(entity.bbWidth + 1.0f) * 0.5;
	double y = point.yCoord;
	double z = point.zCoord + static_cast<int_t>(entity.bbWidth + 1.0f) * 0.5;
	return Vec3::newTemp(x, y, z);
}
