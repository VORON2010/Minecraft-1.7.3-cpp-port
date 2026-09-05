#pragma once

#include "java/Type.h"

class PathPoint
{
public:
	int_t xCoord = 0;
	int_t yCoord = 0;
	int_t zCoord = 0;
	int_t hash = 0;

	int_t index = -1;
	float totalPathDistance = 0.0f;
	float distanceToNext = 0.0f;
	float distanceToTarget = 0.0f;
	PathPoint *previous = nullptr;
	bool isFirst = false;

	PathPoint(int_t xCoord, int_t yCoord, int_t zCoord);

	static int_t hashCoords(int_t xCoord, int_t yCoord, int_t zCoord);
	float distanceTo(const PathPoint &other) const;
	bool equals(const PathPoint &other) const;
	bool isAssigned() const;
};
