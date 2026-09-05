#pragma once

#include "java/Type.h"

#include <array>
#include <memory>
#include <unordered_map>

class Entity;
class LevelSource;
class Path;
class PathEntity;
class PathPoint;

class Pathfinder
{
private:
	LevelSource &worldMap;
	std::unique_ptr<Path> path;
	std::unordered_map<int_t, std::unique_ptr<PathPoint>> pointMap;
	std::array<PathPoint *, 32> pathOptions = {};

	std::unique_ptr<PathEntity> createEntityPathTo(Entity &entity, double x, double y, double z, float distance);
	std::unique_ptr<PathEntity> addToPath(Entity &entity, PathPoint &start, PathPoint &end, PathPoint &size, float distance);
	int_t findPathOptions(Entity &entity, PathPoint &current, PathPoint &size, PathPoint &target, float distance);
	PathPoint *getSafePoint(Entity &entity, int_t x, int_t y, int_t z, PathPoint &size, int_t stepHeight);
	PathPoint *openPoint(int_t x, int_t y, int_t z);
	int_t getVerticalOffset(Entity &entity, int_t x, int_t y, int_t z, PathPoint &size);
	std::unique_ptr<PathEntity> createEntityPath(PathPoint &start, PathPoint &end);

public:
	explicit Pathfinder(LevelSource &worldMap);
	~Pathfinder();

	std::unique_ptr<PathEntity> createEntityPathTo(Entity &start, Entity &end, float distance);
	std::unique_ptr<PathEntity> createEntityPathTo(Entity &start, int_t x, int_t y, int_t z, float distance);
};
