#pragma once

#include <vector>

#include "java/Type.h"

#include "world/level/pathfinder/PathPoint.h"

class Entity;
class Vec3;

class PathEntity
{
private:
	std::vector<PathPoint> points;
	int_t pathIndex = 0;

public:
	int_t pathLength = 0;

	explicit PathEntity(std::vector<PathPoint> &&points);

	void incrementPathIndex();
	bool isFinished() const;
	const PathPoint *getEndPoint() const;
	Vec3 *getPosition(Entity &entity) const;
};
