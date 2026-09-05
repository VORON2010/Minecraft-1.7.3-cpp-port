#pragma once

#include "java/Type.h"

#include <vector>

class PathPoint;

class Path
{
private:
	std::vector<PathPoint *> pathPoints;
	int_t count = 0;

	void sortBack(int_t index);
	void sortForward(int_t index);

public:
	Path();

	PathPoint *addPoint(PathPoint &point);
	void clearPath();
	PathPoint *dequeue();
	void changeDistance(PathPoint &point, float distance);
	bool isPathEmpty() const;
};
