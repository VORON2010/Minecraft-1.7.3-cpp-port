#include "world/level/pathfinder/PathPoint.h"

#include <cstdint>

#include "util/Mth.h"

PathPoint::PathPoint(int_t xCoord, int_t yCoord, int_t zCoord)
	: xCoord(xCoord), yCoord(yCoord), zCoord(zCoord), hash(hashCoords(xCoord, yCoord, zCoord))
{
}

int_t PathPoint::hashCoords(int_t xCoord, int_t yCoord, int_t zCoord)
{
	return (yCoord & 255)
		| ((xCoord & 0x7FFF) << 8)
		| ((zCoord & 0x7FFF) << 24)
		| (xCoord < 0 ? static_cast<int_t>(0x80000000u) : 0)
		| (zCoord < 0 ? 0x8000 : 0);
}

float PathPoint::distanceTo(const PathPoint &other) const
{
	float xDistance = static_cast<float>(other.xCoord - xCoord);
	float yDistance = static_cast<float>(other.yCoord - yCoord);
	float zDistance = static_cast<float>(other.zCoord - zCoord);
	return Mth::sqrt(xDistance * xDistance + yDistance * yDistance + zDistance * zDistance);
}

bool PathPoint::equals(const PathPoint &other) const
{
	return hash == other.hash && xCoord == other.xCoord && yCoord == other.yCoord && zCoord == other.zCoord;
}

bool PathPoint::isAssigned() const
{
	return index >= 0;
}
