#include "world/level/pathfinder/Pathfinder.h"

#include <algorithm>

#include "world/entity/Entity.h"
#include "world/level/LevelSource.h"
#include "world/level/material/Material.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/level/pathfinder/Path.h"
#include "world/level/pathfinder/PathEntity.h"
#include "world/level/pathfinder/PathPoint.h"
#include "world/level/tile/DoorTile.h"
#include "world/level/tile/Tile.h"
#include "util/Mth.h"

Pathfinder::Pathfinder(LevelSource &worldMap) : worldMap(worldMap), path(std::make_unique<Path>())
{
}

Pathfinder::~Pathfinder() = default;

std::unique_ptr<PathEntity> Pathfinder::createEntityPathTo(Entity &start, Entity &end, float distance)
{
	return createEntityPathTo(start, end.x, end.bb.y0, end.z, distance);
}

std::unique_ptr<PathEntity> Pathfinder::createEntityPathTo(Entity &start, int_t x, int_t y, int_t z, float distance)
{
	return createEntityPathTo(start, static_cast<double>(x) + 0.5f, static_cast<double>(y) + 0.5f, static_cast<double>(z) + 0.5f, distance);
}

std::unique_ptr<PathEntity> Pathfinder::createEntityPathTo(Entity &entity, double x, double y, double z, float distance)
{
	path->clearPath();
	pointMap.clear();
	PathPoint *start = openPoint(Mth::floor(entity.bb.x0), Mth::floor(entity.bb.y0), Mth::floor(entity.bb.z0));
	PathPoint *end = openPoint(Mth::floor(x - entity.bbWidth / 2.0f), Mth::floor(y), Mth::floor(z - entity.bbWidth / 2.0f));
	PathPoint size(Mth::floor(entity.bbWidth + 1.0f), Mth::floor(entity.bbHeight + 1.0f), Mth::floor(entity.bbWidth + 1.0f));
	return addToPath(entity, *start, *end, size, distance);
}

std::unique_ptr<PathEntity> Pathfinder::addToPath(Entity &entity, PathPoint &start, PathPoint &end, PathPoint &size, float distance)
{
	start.totalPathDistance = 0.0f;
	start.distanceToNext = start.distanceTo(end);
	start.distanceToTarget = start.distanceToNext;
	path->clearPath();
	path->addPoint(start);
	PathPoint *closest = &start;

	while (!path->isPathEmpty())
	{
		PathPoint *current = path->dequeue();
		if (current->equals(end))
			return createEntityPath(start, end);
		if (current->distanceTo(end) < closest->distanceTo(end))
			closest = current;

		current->isFirst = true;
		int_t optionCount = findPathOptions(entity, *current, size, end, distance);
		for (int_t i = 0; i < optionCount; ++i)
		{
			PathPoint *option = pathOptions[i];
			float totalDistance = current->totalPathDistance + current->distanceTo(*option);
			if (!option->isAssigned() || totalDistance < option->totalPathDistance)
			{
				option->previous = current;
				option->totalPathDistance = totalDistance;
				option->distanceToNext = option->distanceTo(end);
				if (option->isAssigned())
					path->changeDistance(*option, option->totalPathDistance + option->distanceToNext);
				else
				{
					option->distanceToTarget = option->totalPathDistance + option->distanceToNext;
					path->addPoint(*option);
				}
			}
		}
	}

	return closest == &start ? nullptr : createEntityPath(start, *closest);
}

int_t Pathfinder::findPathOptions(Entity &entity, PathPoint &current, PathPoint &size, PathPoint &target, float distance)
{
	int_t optionCount = 0;
	int_t stepHeight = 0;
	if (getVerticalOffset(entity, current.xCoord, current.yCoord + 1, current.zCoord, size) == 1)
		stepHeight = 1;

	PathPoint *south = getSafePoint(entity, current.xCoord, current.yCoord, current.zCoord + 1, size, stepHeight);
	PathPoint *west = getSafePoint(entity, current.xCoord - 1, current.yCoord, current.zCoord, size, stepHeight);
	PathPoint *east = getSafePoint(entity, current.xCoord + 1, current.yCoord, current.zCoord, size, stepHeight);
	PathPoint *north = getSafePoint(entity, current.xCoord, current.yCoord, current.zCoord - 1, size, stepHeight);

	if (south != nullptr && !south->isFirst && south->distanceTo(target) < distance)
		pathOptions[optionCount++] = south;
	if (west != nullptr && !west->isFirst && west->distanceTo(target) < distance)
		pathOptions[optionCount++] = west;
	if (east != nullptr && !east->isFirst && east->distanceTo(target) < distance)
		pathOptions[optionCount++] = east;
	if (north != nullptr && !north->isFirst && north->distanceTo(target) < distance)
		pathOptions[optionCount++] = north;
	return optionCount;
}

PathPoint *Pathfinder::getSafePoint(Entity &entity, int_t x, int_t y, int_t z, PathPoint &size, int_t stepHeight)
{
	PathPoint *point = nullptr;
	if (getVerticalOffset(entity, x, y, z, size) == 1)
		point = openPoint(x, y, z);
	if (point == nullptr && stepHeight > 0 && getVerticalOffset(entity, x, y + stepHeight, z, size) == 1)
	{
		point = openPoint(x, y + stepHeight, z);
		y += stepHeight;
	}

	if (point != nullptr)
	{
		int_t fallDistance = 0;
		int_t verticalOffset = 0;
		while (y > 0)
		{
			verticalOffset = getVerticalOffset(entity, x, y - 1, z, size);
			if (verticalOffset != 1)
				break;
			if (++fallDistance >= 4)
				return nullptr;
			if (--y > 0)
				point = openPoint(x, y, z);
		}

		if (verticalOffset == -2)
			return nullptr;
	}

	return point;
}

PathPoint *Pathfinder::openPoint(int_t x, int_t y, int_t z)
{
	int_t hash = PathPoint::hashCoords(x, y, z);
	auto it = pointMap.find(hash);
	if (it == pointMap.end())
		it = pointMap.emplace(hash, std::make_unique<PathPoint>(x, y, z)).first;
	return it->second.get();
}

int_t Pathfinder::getVerticalOffset(Entity &entity, int_t x, int_t y, int_t z, PathPoint &size)
{
	(void)entity;
	for (int_t dx = x; dx < x + size.xCoord; ++dx)
	{
		for (int_t dy = y; dy < y + size.yCoord; ++dy)
		{
			for (int_t dz = z; dz < z + size.zCoord; ++dz)
			{
				int_t tileId = worldMap.getTile(dx, dy, dz);
				if (tileId <= 0)
					continue;
				if (tileId != Tile::doorIron.id && tileId != Tile::doorWood.id)
				{
					const Material &material = worldMap.getMaterial(dx, dy, dz);
					if (material.isSolid())
						return 0;
					if (&material == &static_cast<const Material &>(Material::water))
						return -1;
					if (&material == &static_cast<const Material &>(Material::lava))
						return -2;
				}
				else if ((worldMap.getData(dx, dy, dz) & 4) == 0)
				{
					return 0;
				}
			}
		}
	}

	return 1;
}

std::unique_ptr<PathEntity> Pathfinder::createEntityPath(PathPoint &start, PathPoint &end)
{
	std::vector<PathPoint> points;
	for (PathPoint *point = &end; point != nullptr; point = point->previous)
		points.push_back(*point);
	std::reverse(points.begin(), points.end());
	return std::make_unique<PathEntity>(std::move(points));
}
