#include "world/entity/ai/PathNavigation.h"

#include <cmath>

#include "world/entity/Mob.h"
#include "world/entity/ai/Controls.h"
#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/LiquidTile.h"
#include "world/level/tile/DoorTile.h"
#include "world/level/tile/TrapDoorTile.h"
#include "world/level/tile/FenceGateTile.h"
#include "util/Mth.h"

// ---- Node ----

int_t Node::createHash(int_t x, int_t y, int_t z)
{
	return (y & 0xFF) | ((x & 32767) << 8) | ((z & 32767) << 24) | (x < 0 ? static_cast<int_t>(0x80000000u) : 0) | (z < 0 ? 32768 : 0);
}

float Node::distanceTo(const Node &o) const
{
	float xd = static_cast<float>(o.x - x), yd = static_cast<float>(o.y - y), zd = static_cast<float>(o.z - z);
	return Mth::sqrt(xd * xd + yd * yd + zd * zd);
}

float Node::distanceToSqr(const Node &o) const
{
	float xd = static_cast<float>(o.x - x), yd = static_cast<float>(o.y - y), zd = static_cast<float>(o.z - z);
	return xd * xd + yd * yd + zd * zd;
}

// ---- NodeHeap ----

Node *NodeHeap::insert(Node *node)
{
	if (size == static_cast<int_t>(heap.size()))
		heap.resize(size << 1);
	heap[size] = node;
	node->heapIdx = size;
	upHeap(size++);
	return node;
}

Node *NodeHeap::pop()
{
	Node *top = heap[0];
	heap[0] = heap[--size];
	heap[size] = nullptr;
	if (size > 0)
		downHeap(0);
	top->heapIdx = -1;
	return top;
}

void NodeHeap::changeCost(Node *node, float cost)
{
	float old = node->f;
	node->f = cost;
	if (cost < old)
		upHeap(node->heapIdx);
	else
		downHeap(node->heapIdx);
}

void NodeHeap::upHeap(int_t idx)
{
	Node *node = heap[idx];
	float cost = node->f;
	while (idx > 0)
	{
		int_t parentIdx = (idx - 1) >> 1;
		Node *parent = heap[parentIdx];
		if (!(cost < parent->f))
			break;
		heap[idx] = parent;
		parent->heapIdx = idx;
		idx = parentIdx;
	}
	heap[idx] = node;
	node->heapIdx = idx;
}

void NodeHeap::downHeap(int_t idx)
{
	Node *node = heap[idx];
	float cost = node->f;
	while (true)
	{
		int_t leftIdx = 1 + (idx << 1);
		int_t rightIdx = leftIdx + 1;
		if (leftIdx >= size)
			break;
		Node *left = heap[leftIdx];
		float leftCost = left->f;
		Node *right;
		float rightCost;
		if (rightIdx >= size)
		{
			right = nullptr;
			rightCost = INFINITY;
		}
		else
		{
			right = heap[rightIdx];
			rightCost = right->f;
		}
		if (leftCost < rightCost)
		{
			if (!(leftCost < cost))
				break;
			heap[idx] = left;
			left->heapIdx = idx;
			idx = leftIdx;
		}
		else
		{
			if (!(rightCost < cost))
				break;
			heap[idx] = right;
			right->heapIdx = idx;
			idx = rightIdx;
		}
	}
	heap[idx] = node;
	node->heapIdx = idx;
}

// ---- NavPath ----

NavPath::NavPath(std::vector<std::unique_ptr<Node>> nodes) : nodes(std::move(nodes))
{
	length = static_cast<int_t>(this->nodes.size());
}

Vec3 NavPath::getPos(Entity &entity, int_t i) const
{
	double off = static_cast<int_t>(entity.bbWidth + 1.0f) * 0.5;
	return Vec3(nodes[i]->x + off, nodes[i]->y, nodes[i]->z + off);
}

bool NavPath::sameAs(const NavPath *other) const
{
	if (other == nullptr || other->nodes.size() != nodes.size())
		return false;
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i]->x != other->nodes[i]->x || nodes[i]->y != other->nodes[i]->y || nodes[i]->z != other->nodes[i]->z)
			return false;
	}
	return true;
}

bool NavPath::endsInXZ(const Vec3 &pos) const
{
	Node *end = last();
	return end != nullptr && end->x == static_cast<int_t>(pos.x) && end->z == static_cast<int_t>(pos.z);
}

// ---- NodePathFinder ----

std::unique_ptr<NavPath> NodePathFinder::findPath(Entity &from, Entity &to, float maxDist)
{
	return findPath(from, to.x, to.bb.y0, to.z, maxDist);
}

std::unique_ptr<NavPath> NodePathFinder::findPath(Entity &from, int_t x, int_t y, int_t z, float maxDist)
{
	return findPath(from, x + 0.5f, y + 0.5f, z + 0.5f, maxDist);
}

std::unique_ptr<NavPath> NodePathFinder::findPath(Entity &entity, double x, double y, double z, float maxDist)
{
	openSet.clear();
	nodes.clear();
	bool oldAvoidWater = avoidWater;
	int_t startY = Mth::floor(entity.bb.y0 + 0.5);
	if (canFloat && entity.isInWater())
	{
		startY = static_cast<int_t>(entity.bb.y0);
		int_t tile = level.getTile(Mth::floor(entity.x), startY, Mth::floor(entity.z));
		while (tile == Tile::water.id || tile == Tile::calmWater.id)
		{
			++startY;
			tile = level.getTile(Mth::floor(entity.x), startY, Mth::floor(entity.z));
		}
		oldAvoidWater = avoidWater;
		avoidWater = false;
	}
	else
		startY = Mth::floor(entity.bb.y0 + 0.5);

	Node *from = getNode(Mth::floor(entity.bb.x0), startY, Mth::floor(entity.bb.z0));
	Node *to = getNode(Mth::floor(x - entity.bbWidth / 2.0f), Mth::floor(y), Mth::floor(z - entity.bbWidth / 2.0f));
	Node size(Mth::floor(entity.bbWidth + 1.0f), Mth::floor(entity.bbHeight + 1.0f), Mth::floor(entity.bbWidth + 1.0f));
	std::unique_ptr<NavPath> path = findPath(entity, from, to, size, maxDist);
	avoidWater = oldAvoidWater;
	return path;
}

std::unique_ptr<NavPath> NodePathFinder::findPath(Entity &entity, Node *from, Node *to, Node &size, float maxDist)
{
	from->g = 0.0f;
	from->h = from->distanceToSqr(*to);
	from->f = from->h;
	openSet.clear();
	openSet.insert(from);
	Node *closest = from;
	while (!openSet.isEmpty())
	{
		Node *x = openSet.pop();
		if (x->same(*to))
			return reconstructPath(from, to);
		if (x->distanceToSqr(*to) < closest->distanceToSqr(*to))
			closest = x;
		x->closed = true;
		int_t count = getNeighbors(entity, *x, size, *to, maxDist);
		for (int_t i = 0; i < count; ++i)
		{
			Node *n = neighbors[i];
			float g = x->g + x->distanceToSqr(*n);
			if (!n->inOpenSet() || g < n->g)
			{
				n->cameFrom = x;
				n->g = g;
				n->h = n->distanceToSqr(*to);
				if (n->inOpenSet())
					openSet.changeCost(n, n->g + n->h);
				else
				{
					n->f = n->g + n->h;
					openSet.insert(n);
				}
			}
		}
	}
	if (closest == from)
		return nullptr;
	return reconstructPath(from, closest);
}

int_t NodePathFinder::getNeighbors(Entity &entity, Node &pos, Node &size, Node &target, float maxDist)
{
	int_t count = 0;
	int_t jumpSize = 0;
	if (isFree(entity, pos.x, pos.y + 1, pos.z, size) == 1)
		jumpSize = 1;
	Node *n = getNode(entity, pos.x, pos.y, pos.z + 1, size, jumpSize);
	Node *w = getNode(entity, pos.x - 1, pos.y, pos.z, size, jumpSize);
	Node *e = getNode(entity, pos.x + 1, pos.y, pos.z, size, jumpSize);
	Node *s = getNode(entity, pos.x, pos.y, pos.z - 1, size, jumpSize);
	for (Node *c : {n, w, e, s})
	{
		if (c != nullptr && !c->closed && c->distanceTo(target) < maxDist)
			neighbors[count++] = c;
	}
	return count;
}

Node *NodePathFinder::getNode(Entity &entity, int_t x, int_t y, int_t z, Node &size, int_t jumpSize)
{
	Node *best = nullptr;
	int_t pathType = isFree(entity, x, y, z, size);
	if (pathType == 2)
		return getNode(x, y, z);
	if (pathType == 1)
		best = getNode(x, y, z);
	if (best == nullptr && jumpSize > 0 && pathType != -3 && pathType != -4 && isFree(entity, x, y + jumpSize, z, size) == 1)
	{
		best = getNode(x, y + jumpSize, z);
		y += jumpSize;
	}

	if (best != nullptr)
	{
		int_t drop = 0;
		int_t cost = 0;
		while (y > 0)
		{
			cost = isFree(entity, x, y - 1, z, size);
			if (avoidWater && cost == -1)
				return nullptr;
			if (cost != 1)
				break;
			if (drop++ >= entity.getMaxFallDistance())
				return nullptr;
			if (--y > 0)
				best = getNode(x, y, z);
		}
		if (cost == -2)
			return nullptr;
	}
	return best;
}

Node *NodePathFinder::getNode(int_t x, int_t y, int_t z)
{
	int_t hash = Node::createHash(x, y, z);
	auto it = nodes.find(hash);
	if (it != nodes.end())
		return it->second.get();
	auto node = std::make_unique<Node>(x, y, z);
	Node *raw = node.get();
	nodes[hash] = std::move(node);
	return raw;
}

int_t NodePathFinder::isFree(Entity &entity, int_t x, int_t y, int_t z, Node &size)
{
	return isFree(entity, x, y, z, size, avoidWater, openDoors, passDoors);
}

int_t NodePathFinder::isFree(Entity &entity, int_t x, int_t y, int_t z, Node &size, bool avoidWater, bool canOpenDoors, bool canPassDoors)
{
	bool walkable = false;
	Level &level = entity.level;
	for (int_t xx = x; xx < x + size.x; ++xx)
	{
		for (int_t yy = y; yy < y + size.y; ++yy)
		{
			for (int_t zz = z; zz < z + size.z; ++zz)
			{
				int_t tileId = level.getTile(xx, yy, zz);
				if (tileId <= 0)
					continue;
				if (tileId == Tile::trapdoor.id)
					walkable = true;
				else if (tileId != Tile::water.id && tileId != Tile::calmWater.id)
				{
					if (!canPassDoors && tileId == Tile::doorWood.id)
						return 0;
				}
				else
				{
					if (avoidWater)
						return -1;
					walkable = true;
				}

				Tile *tile = Tile::tiles[tileId];
				int_t shape = tile->getRenderShape();
				if (Tile::tiles[level.getTile(xx, yy, zz)]->getRenderShape() == Tile::SHAPE_RAIL)
				{
					int_t ex = Mth::floor(entity.x);
					int_t ey = Mth::floor(entity.y);
					int_t ez = Mth::floor(entity.z);
					auto railAt = [&](int_t px, int_t py, int_t pz) {
						int_t t = level.getTile(px, py, pz);
						return t > 0 && Tile::tiles[t] != nullptr && Tile::tiles[t]->getRenderShape() == Tile::SHAPE_RAIL;
					};
					if (!railAt(ex, ey, ez) && !railAt(ex, ey - 1, ez))
						return -3;
				}
				else if (!tile->isPathfindable(level, xx, yy, zz) && (!canOpenDoors || tileId != Tile::doorWood.id))
				{
					if (shape == Tile::SHAPE_FENCE || tileId == Tile::fenceGate.id || shape == 32)
						return -3;
					if (tileId == Tile::trapdoor.id)
						return -4;
					if (&tile->material != &Material::lava)
						return 0;
					if (!entity.isInLava())
						return -2;
				}
			}
		}
	}
	return walkable ? 2 : 1;
}

std::unique_ptr<NavPath> NodePathFinder::reconstructPath(Node *from, Node *to)
{
	(void)from;
	std::vector<Node *> chain;
	for (Node *n = to; n != nullptr; n = n->cameFrom)
		chain.push_back(n);
	std::vector<std::unique_ptr<Node>> copies;
	for (auto it = chain.rbegin(); it != chain.rend(); ++it)
		copies.push_back(std::make_unique<Node>((*it)->x, (*it)->y, (*it)->z));
	return std::make_unique<NavPath>(std::move(copies));
}

// ---- PathNavigation ----

std::shared_ptr<NavPath> PathNavigation::createPath(double x, double y, double z)
{
	if (!canUpdatePath())
		return nullptr;
	return level.findPath(mob, Mth::floor(x), static_cast<int_t>(y), Mth::floor(z), maxDist, canPassDoors, canOpenDoors, avoidWater, canFloat);
}

bool PathNavigation::moveTo(double x, double y, double z, float speed)
{
	std::shared_ptr<NavPath> newPath = createPath(Mth::floor(x), static_cast<int_t>(y), Mth::floor(z));
	return moveTo(newPath, speed);
}

std::shared_ptr<NavPath> PathNavigation::createPath(Mob &target)
{
	if (!canUpdatePath())
		return nullptr;
	return level.findPath(mob, target, maxDist, canPassDoors, canOpenDoors, avoidWater, canFloat);
}

bool PathNavigation::moveTo(Mob &target, float speed)
{
	std::shared_ptr<NavPath> newPath = createPath(target);
	return newPath != nullptr ? moveTo(newPath, speed) : false;
}

bool PathNavigation::moveTo(std::shared_ptr<NavPath> newPath, float speed)
{
	if (newPath == nullptr)
	{
		path = nullptr;
		return false;
	}
	if (!newPath->sameAs(path.get()))
		path = newPath;
	if (avoidSun)
		trimPathFromSun();
	if (path->getSize() == 0)
		return false;
	this->speed = speed;
	Vec3 pos = getTempMobPos();
	lastStuckCheck = tick_;
	lastStuckCheckPos = pos;
	return true;
}

void PathNavigation::tick()
{
	++tick_;
	if (isDone())
		return;
	if (canUpdatePath())
		updatePath();
	if (isDone())
		return;
	Vec3 target = path->currentPos(mob);
	mob.getMoveControl().setWantedPosition(target.x, target.y, target.z, speed);
}

void PathNavigation::updatePath()
{
	Vec3 mobPos = getTempMobPos();
	int_t firstElevation = path->getSize();
	for (int_t i = path->getIndex(); i < path->getSize(); ++i)
	{
		if (path->get(i)->y != static_cast<int_t>(mobPos.y))
		{
			firstElevation = i;
			break;
		}
	}

	float maxDistSqr = mob.bbWidth * mob.bbWidth;
	for (int_t i = path->getIndex(); i < firstElevation; ++i)
	{
		if (mobPos.distanceToSqr(path->getPos(mob, i)) < maxDistSqr)
			path->setIndex(i + 1);
	}

	int_t sx = Mth::ceil(mob.bbWidth);
	int_t sy = static_cast<int_t>(mob.bbHeight) + 1;
	int_t sz = sx;
	for (int_t i = firstElevation - 1; i >= path->getIndex(); --i)
	{
		if (canMoveDirectly(mobPos, path->getPos(mob, i), sx, sy, sz))
		{
			path->setIndex(i);
			break;
		}
	}

	if (tick_ - lastStuckCheck > 100)
	{
		if (mobPos.distanceToSqr(lastStuckCheckPos) < 2.25)
			stop();
		lastStuckCheck = tick_;
		lastStuckCheckPos = mobPos;
	}
}

Vec3 PathNavigation::getTempMobPos()
{
	return Vec3(mob.x, getSurfaceY(), mob.z);
}

int_t PathNavigation::getSurfaceY()
{
	if (mob.isInWater() && canFloat)
	{
		int_t surface = static_cast<int_t>(mob.bb.y0);
		int_t tile = level.getTile(Mth::floor(mob.x), surface, Mth::floor(mob.z));
		int_t steps = 0;
		while (tile == Tile::water.id || tile == Tile::calmWater.id)
		{
			++surface;
			tile = level.getTile(Mth::floor(mob.x), surface, Mth::floor(mob.z));
			if (++steps > 16)
				return static_cast<int_t>(mob.bb.y0);
		}
		return surface;
	}
	return static_cast<int_t>(mob.bb.y0 + 0.5);
}

bool PathNavigation::canUpdatePath()
{
	return mob.onGround || (canFloat && isInLiquid());
}

bool PathNavigation::isInLiquid()
{
	return mob.isInWater() || mob.isInLava();
}

void PathNavigation::trimPathFromSun()
{
	if (level.canSeeSky(Mth::floor(mob.x), static_cast<int_t>(mob.bb.y0 + 0.5), Mth::floor(mob.z)))
		return;
	for (int_t i = 0; i < path->getSize(); ++i)
	{
		Node *n = path->get(i);
		if (level.canSeeSky(n->x, n->y, n->z))
		{
			path->setSize(i - 1);
			return;
		}
	}
}

bool PathNavigation::canMoveDirectly(const Vec3 &from, const Vec3 &to, int_t sx, int_t sy, int_t sz)
{
	int_t gridX = Mth::floor(from.x);
	int_t gridZ = Mth::floor(from.z);
	double dirX = to.x - from.x;
	double dirZ = to.z - from.z;
	double distSqr = dirX * dirX + dirZ * dirZ;
	if (distSqr < 1.0E-8)
		return false;
	double nf = 1.0 / std::sqrt(distSqr);
	dirX *= nf;
	dirZ *= nf;
	sx += 2;
	sz += 2;
	if (!canWalkOn(gridX, static_cast<int_t>(from.y), gridZ, sx, sy, sz, from, dirX, dirZ))
		return false;
	sx -= 2;
	sz -= 2;
	double deltaX = 1.0 / std::abs(dirX);
	double deltaZ = 1.0 / std::abs(dirZ);
	double maxX = gridX - from.x;
	double maxZ = gridZ - from.z;
	if (dirX >= 0.0)
		maxX++;
	if (dirZ >= 0.0)
		maxZ++;
	maxX /= dirX;
	maxZ /= dirZ;
	int_t stepX = dirX < 0.0 ? -1 : 1;
	int_t stepZ = dirZ < 0.0 ? -1 : 1;
	int_t gridGoalX = Mth::floor(to.x);
	int_t gridGoalZ = Mth::floor(to.z);
	int_t currentDirX = gridGoalX - gridX;
	int_t currentDirZ = gridGoalZ - gridZ;
	while (currentDirX * stepX > 0 || currentDirZ * stepZ > 0)
	{
		if (maxX < maxZ)
		{
			maxX += deltaX;
			gridX += stepX;
			currentDirX = gridGoalX - gridX;
		}
		else
		{
			maxZ += deltaZ;
			gridZ += stepZ;
			currentDirZ = gridGoalZ - gridZ;
		}
		if (!canWalkOn(gridX, static_cast<int_t>(from.y), gridZ, sx, sy, sz, from, dirX, dirZ))
			return false;
	}
	return true;
}

bool PathNavigation::canWalkOn(int_t x, int_t y, int_t z, int_t sx, int_t sy, int_t sz, const Vec3 &start, double dx, double dz)
{
	int_t x0 = x - sx / 2;
	int_t z0 = z - sz / 2;
	if (!canWalkAbove(x0, y, z0, sx, sy, sz, start, dx, dz))
		return false;
	for (int_t xx = x0; xx < x0 + sx; ++xx)
	{
		for (int_t zz = z0; zz < z0 + sz; ++zz)
		{
			double ox = xx + 0.5 - start.x;
			double oz = zz + 0.5 - start.z;
			if (ox * dx + oz * dz < 0.0)
				continue;
			int_t tile = level.getTile(xx, y - 1, zz);
			if (tile <= 0)
				return false;
			const Material &m = Tile::tiles[tile]->material;
			if (&m == &Material::water && !mob.isInWater())
				return false;
			if (&m == &Material::lava)
				return false;
		}
	}
	return true;
}

bool PathNavigation::canWalkAbove(int_t x, int_t y, int_t z, int_t sx, int_t sy, int_t sz, const Vec3 &start, double dx, double dz)
{
	for (int_t xx = x; xx < x + sx; ++xx)
	{
		for (int_t yy = y; yy < y + sy; ++yy)
		{
			for (int_t zz = z; zz < z + sz; ++zz)
			{
				double ox = xx + 0.5 - start.x;
				double oz = zz + 0.5 - start.z;
				if (ox * dx + oz * dz < 0.0)
					continue;
				int_t tile = level.getTile(xx, yy, zz);
				if (tile > 0 && !Tile::tiles[tile]->isPathfindable(level, xx, yy, zz))
					return false;
			}
		}
	}
	return true;
}
