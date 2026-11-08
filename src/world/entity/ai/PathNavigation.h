#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "java/Type.h"
#include "world/phys/Vec3.h"

class Entity;
class Mob;
class Level;

// 1.5.2 PathNode
struct Node
{
	const int_t x, y, z;
	const int_t hash;
	int_t heapIdx = -1;
	float g = 0.0f, h = 0.0f, f = 0.0f;
	Node *cameFrom = nullptr;
	bool closed = false;

	Node(int_t x, int_t y, int_t z) : x(x), y(y), z(z), hash(createHash(x, y, z)) {}
	static int_t createHash(int_t x, int_t y, int_t z);
	float distanceTo(const Node &other) const;
	float distanceToSqr(const Node &other) const;
	bool inOpenSet() const { return heapIdx >= 0; }
	bool same(const Node &o) const { return hash == o.hash && x == o.x && y == o.y && z == o.z; }
};

// 1.5.2 BinaryHeap
class NodeHeap
{
	std::vector<Node *> heap = std::vector<Node *>(1024);
	int_t size = 0;

	void upHeap(int_t idx);
	void downHeap(int_t idx);

public:
	Node *insert(Node *node);
	void clear() { size = 0; }
	Node *pop();
	void changeCost(Node *node, float cost);
	bool isEmpty() const { return size == 0; }
};

// 1.5.2 Path: the nodes are copies, so the path outlives the finder
class NavPath
{
	std::vector<std::unique_ptr<Node>> nodes;
	int_t index = 0;
	int_t length = 0;

public:
	explicit NavPath(std::vector<std::unique_ptr<Node>> nodes);
	void next() { index++; }
	bool isDone() const { return index >= length; }
	Node *last() const { return length > 0 ? nodes[length - 1].get() : nullptr; }
	Node *get(int_t i) const { return nodes[i].get(); }
	int_t getSize() const { return length; }
	void setSize(int_t value) { length = value; }
	int_t getIndex() const { return index; }
	void setIndex(int_t value) { index = value; }
	Vec3 getPos(Entity &entity, int_t i) const;
	Vec3 currentPos(Entity &entity) const { return getPos(entity, index); }
	bool sameAs(const NavPath *other) const;
	bool endsInXZ(const Vec3 &pos) const;
};

// 1.5.2 PathFinder
class NodePathFinder
{
	Level &level;
	NodeHeap openSet;
	std::unordered_map<int_t, std::unique_ptr<Node>> nodes;
	Node *neighbors[32] = {};
	bool passDoors;
	bool openDoors;
	bool avoidWater;
	bool canFloat;

	std::unique_ptr<NavPath> findPath(Entity &entity, double x, double y, double z, float maxDist);
	std::unique_ptr<NavPath> findPath(Entity &entity, Node *from, Node *to, Node &size, float maxDist);
	int_t getNeighbors(Entity &entity, Node &pos, Node &size, Node &target, float maxDist);
	Node *getNode(Entity &entity, int_t x, int_t y, int_t z, Node &size, int_t jumpSize);
	Node *getNode(int_t x, int_t y, int_t z);
	std::unique_ptr<NavPath> reconstructPath(Node *from, Node *to);

public:
	NodePathFinder(Level &level, bool passDoors, bool openDoors, bool avoidWater, bool canFloat)
		: level(level), passDoors(passDoors), openDoors(openDoors), avoidWater(avoidWater), canFloat(canFloat) {}

	std::unique_ptr<NavPath> findPath(Entity &from, Entity &to, float maxDist);
	std::unique_ptr<NavPath> findPath(Entity &from, int_t x, int_t y, int_t z, float maxDist);
	int_t isFree(Entity &entity, int_t x, int_t y, int_t z, Node &size);
	static int_t isFree(Entity &entity, int_t x, int_t y, int_t z, Node &size, bool avoidWater, bool canOpenDoors, bool canPassDoors);
};

// 1.5.2 PathNavigation
class PathNavigation
{
	Mob &mob;
	Level &level;
	std::shared_ptr<NavPath> path;
	float speed = 0.0f;
	float maxDist;
	bool avoidSun = false;
	int_t tick_ = 0;
	int_t lastStuckCheck = 0;
	Vec3 lastStuckCheckPos{0.0, 0.0, 0.0};
	bool canPassDoors = true;
	bool canOpenDoors = false;
	bool avoidWater = false;
	bool canFloat = false;

	void updatePath();
	Vec3 getTempMobPos();
	int_t getSurfaceY();
	bool canUpdatePath();
	bool isInLiquid();
	void trimPathFromSun();
	bool canMoveDirectly(const Vec3 &from, const Vec3 &to, int_t sx, int_t sy, int_t sz);
	bool canWalkOn(int_t x, int_t y, int_t z, int_t sx, int_t sy, int_t sz, const Vec3 &start, double dx, double dz);
	bool canWalkAbove(int_t x, int_t y, int_t z, int_t sx, int_t sy, int_t sz, const Vec3 &start, double dx, double dz);

public:
	PathNavigation(Mob &mob, Level &level, float maxDist) : mob(mob), level(level), maxDist(maxDist) {}

	void setAvoidWater(bool value) { avoidWater = value; }
	bool getAvoidWater() const { return avoidWater; }
	void setCanOpenDoors(bool value) { canOpenDoors = value; }
	void setCanPassDoors(bool value) { canPassDoors = value; }
	bool getCanOpenDoors() const { return canOpenDoors; }
	void setAvoidSun(bool value) { avoidSun = value; }
	void setSpeed(float value) { speed = value; }
	void setCanFloat(bool value) { canFloat = value; }

	std::shared_ptr<NavPath> createPath(double x, double y, double z);
	bool moveTo(double x, double y, double z, float speed);
	std::shared_ptr<NavPath> createPath(Mob &target);
	bool moveTo(Mob &target, float speed);
	bool moveTo(std::shared_ptr<NavPath> newPath, float speed);
	NavPath *getPath() const { return path.get(); }
	void tick();
	bool isDone() const { return path == nullptr || path->isDone(); }
	void stop() { path = nullptr; }
};
