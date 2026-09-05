#include "world/level/pathfinder/Path.h"

#include <limits>
#include <stdexcept>

#include "world/level/pathfinder/PathPoint.h"

Path::Path()
{
	pathPoints.resize(1024, nullptr);
}

PathPoint *Path::addPoint(PathPoint &point)
{
	if (point.index >= 0)
		throw std::logic_error("point already assigned");
	if (count == static_cast<int_t>(pathPoints.size()))
		pathPoints.resize(pathPoints.size() << 1, nullptr);

	pathPoints[count] = &point;
	point.index = count;
	sortBack(count++);
	return &point;
}

void Path::clearPath()
{
	count = 0;
}

PathPoint *Path::dequeue()
{
	PathPoint *point = pathPoints[0];
	pathPoints[0] = pathPoints[--count];
	pathPoints[count] = nullptr;
	if (count > 0)
		sortForward(0);
	point->index = -1;
	return point;
}

void Path::changeDistance(PathPoint &point, float distance)
{
	float previousDistance = point.distanceToTarget;
	point.distanceToTarget = distance;
	if (distance < previousDistance)
		sortBack(point.index);
	else
		sortForward(point.index);
}

void Path::sortBack(int_t index)
{
	PathPoint *point = pathPoints[index];
	float distance = point->distanceToTarget;
	while (index > 0)
	{
		int_t parentIndex = (index - 1) >> 1;
		PathPoint *parent = pathPoints[parentIndex];
		if (!(distance < parent->distanceToTarget))
			break;
		pathPoints[index] = parent;
		parent->index = index;
		index = parentIndex;
	}
	pathPoints[index] = point;
	point->index = index;
}

void Path::sortForward(int_t index)
{
	PathPoint *point = pathPoints[index];
	float distance = point->distanceToTarget;
	while (true)
	{
		int_t leftChild = 1 + (index << 1);
		int_t rightChild = leftChild + 1;
		if (leftChild >= count)
			break;

		PathPoint *leftPoint = pathPoints[leftChild];
		float leftDistance = leftPoint->distanceToTarget;
		PathPoint *rightPoint = rightChild >= count ? nullptr : pathPoints[rightChild];
		float rightDistance = rightPoint == nullptr ? std::numeric_limits<float>::infinity() : rightPoint->distanceToTarget;

		if (leftDistance < rightDistance)
		{
			if (!(leftDistance < distance))
				break;
			pathPoints[index] = leftPoint;
			leftPoint->index = index;
			index = leftChild;
		}
		else
		{
			if (!(rightDistance < distance))
				break;
			pathPoints[index] = rightPoint;
			rightPoint->index = index;
			index = rightChild;
		}
	}
	pathPoints[index] = point;
	point->index = index;
}

bool Path::isPathEmpty() const
{
	return count == 0;
}
