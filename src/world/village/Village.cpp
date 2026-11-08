#include "world/village/Village.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "nbt/CompoundTag.h"
#include "nbt/ListTag.h"
#include "world/entity/EntityIO.h"
#include "world/entity/Mob.h"
#include "world/entity/npc/Villager.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/DoorTile.h"
#include "world/phys/AABB.h"
#include "util/Mth.h"
#include "util/Memory.h"

// ---- DoorInfo ----

int_t DoorInfo::distanceToSqr(int_t xx, int_t yy, int_t zz) const
{
	int_t dx = xx - x, dy = yy - y, dz = zz - z;
	return dx * dx + dy * dy + dz * dz;
}

int_t DoorInfo::distanceToInsideSqr(int_t xx, int_t yy, int_t zz) const
{
	int_t dx = xx - x - insideDx, dy = yy - y, dz = zz - z - insideDz;
	return dx * dx + dy * dy + dz * dz;
}

bool DoorInfo::isInsideSide(int_t xx, int_t zz) const
{
	int_t dx = xx - x, dz = zz - z;
	return dx * insideDx + dz * insideDz >= 0;
}

// ---- Village ----

static bool isGolem(Entity &entity)
{
	return entity.getEncodeId() == u"VillagerGolem";
}

void Village::tick(int_t tick)
{
	tick_ = tick;
	updateDoors();
	updateAggressors();
	if (tick % 20 == 0)
		countPopulation();
	if (tick % 30 == 0)
		countGolem();

	int_t idealGolems = populationSize / 10;
	if (golemCount < idealGolems && static_cast<int_t>(doorInfos.size()) > 20 && level->random.nextInt(7000) == 0)
	{
		for (int_t i = 0; i < 10; ++i)
		{
			int_t xx = center.x + level->random.nextInt(16) - 8;
			int_t yy = center.y + level->random.nextInt(6) - 3;
			int_t zz = center.z + level->random.nextInt(16) - 8;
			if (isInside(xx, yy, zz) && canSpawnAt(xx, yy, zz, 2, 4, 2))
			{
				std::shared_ptr<Entity> golem = EntityIO::newEntity(u"VillagerGolem", *level);
				if (golem != nullptr)
				{
					golem->setPos(xx, yy, zz);
					level->addEntity(golem);
					golemCount++;
				}
				break;
			}
		}
	}
}

bool Village::canSpawnAt(int_t x, int_t y, int_t z, int_t sx, int_t sy, int_t sz)
{
	if (!level->isSolidTile(x, y - 1, z))
		return false;
	int_t startX = x - sx / 2;
	int_t startZ = z - sz / 2;
	for (int_t xx = startX; xx < startX + sx; ++xx)
		for (int_t yy = y; yy < y + sy; ++yy)
			for (int_t zz = startZ; zz < startZ + sz; ++zz)
				if (level->isBlockNormalCube(xx, yy, zz))
					return false;
	return true;
}

void Village::countGolem()
{
	AABB *box = AABB::newTemp(center.x - radius, center.y - 4, center.z - radius, center.x + radius, center.y + 4, center.z + radius);
	int_t count = 0;
	for (const auto &e : level->getEntities(nullptr, *box))
		if (isGolem(*e))
			count++;
	golemCount = count;
}

void Village::countPopulation()
{
	AABB *box = AABB::newTemp(center.x - radius, center.y - 4, center.z - radius, center.x + radius, center.y + 4, center.z + radius);
	int_t count = 0;
	for (const auto &e : level->getEntities(nullptr, *box))
		if (dynamic_cast<Villager *>(e.get()) != nullptr)
			count++;
	populationSize = count;
	if (populationSize == 0)
		playerStanding.clear();
}

bool Village::isInside(int_t x, int_t y, int_t z) const
{
	return tilePosDistSqr(center, x, y, z) < static_cast<float>(radius * radius);
}

DoorInfo *Village::getClosestDoorInfo(int_t x, int_t y, int_t z)
{
	DoorInfo *closest = nullptr;
	int_t closestDist = std::numeric_limits<int_t>::max();
	for (auto &d : doorInfos)
	{
		int_t dist = d->distanceToSqr(x, y, z);
		if (dist < closestDist)
		{
			closest = d.get();
			closestDist = dist;
		}
	}
	return closest;
}

DoorInfo *Village::getBestDoorInfo(int_t x, int_t y, int_t z)
{
	DoorInfo *closest = nullptr;
	int_t closestDist = std::numeric_limits<int_t>::max();
	for (auto &d : doorInfos)
	{
		int_t dist = d->distanceToSqr(x, y, z);
		if (dist > 256)
			dist *= 1000;
		else
			dist = d->getBookingsCount();
		if (dist < closestDist)
		{
			closest = d.get();
			closestDist = dist;
		}
	}
	return closest;
}

DoorInfo *Village::getDoorInfo(int_t x, int_t y, int_t z)
{
	if (tilePosDistSqr(center, x, y, z) > static_cast<float>(radius * radius))
		return nullptr;
	for (auto &d : doorInfos)
		if (d->x == x && d->z == z && std::abs(d->y - y) <= 1)
			return d.get();
	return nullptr;
}

void Village::addDoorInfo(std::unique_ptr<DoorInfo> door)
{
	accCenter.x += door->x;
	accCenter.y += door->y;
	accCenter.z += door->z;
	stableSince = door->timeStamp;
	doorInfos.push_back(std::move(door));
	calcInfo();
}

void Village::addAggressor(const std::shared_ptr<Mob> &mob)
{
	for (auto &a : aggressors)
	{
		if (a.mob.lock() == mob)
		{
			a.timeStamp = tick_;
			return;
		}
	}
	aggressors.push_back({mob, tick_});
}

std::shared_ptr<Mob> Village::getClosestAggressor(Mob &from)
{
	double closestSqr = std::numeric_limits<double>::max();
	std::shared_ptr<Mob> closest;
	for (auto &a : aggressors)
	{
		std::shared_ptr<Mob> mob = a.mob.lock();
		if (mob == nullptr)
			continue;
		double distSqr = mob->distanceToSqr(from);
		if (!(distSqr > closestSqr))
		{
			closest = mob;
			closestSqr = distSqr;
		}
	}
	return closest;
}

std::shared_ptr<Player> Village::getClosestBadStandingPlayer(Mob &from)
{
	double closestSqr = std::numeric_limits<double>::max();
	std::shared_ptr<Player> closest;
	for (const auto &standing : playerStanding)
	{
		if (!isVeryBadStanding(standing.first))
			continue;
		for (const auto &player : level->players)
		{
			if (player->name != standing.first)
				continue;
			double distSqr = player->distanceToSqr(from);
			if (!(distSqr > closestSqr))
			{
				closest = player;
				closestSqr = distSqr;
			}
		}
	}
	return closest;
}

void Village::updateAggressors()
{
	for (auto it = aggressors.begin(); it != aggressors.end();)
	{
		std::shared_ptr<Mob> mob = it->mob.lock();
		if (mob == nullptr || !mob->isAlive() || std::abs(tick_ - it->timeStamp) > 300)
			it = aggressors.erase(it);
		else
			++it;
	}
}

void Village::updateDoors()
{
	bool removed = false;
	bool resetBookings = level->random.nextInt(50) == 0;
	for (auto it = doorInfos.begin(); it != doorInfos.end();)
	{
		DoorInfo &door = **it;
		if (resetBookings)
			door.resetBookingCount();
		if (!isDoor(door.x, door.y, door.z) || std::abs(tick_ - door.timeStamp) > 1200)
		{
			accCenter.x -= door.x;
			accCenter.y -= door.y;
			accCenter.z -= door.z;
			removed = true;
			door.removed = true;
			it = doorInfos.erase(it);
		}
		else
			++it;
	}
	if (removed)
		calcInfo();
}

bool Village::isDoor(int_t x, int_t y, int_t z)
{
	int_t tile = level->getTile(x, y, z);
	return tile > 0 && tile == Tile::doorWood.id;
}

void Village::calcInfo()
{
	int_t count = static_cast<int_t>(doorInfos.size());
	if (count == 0)
	{
		center = TilePos(0, 0, 0);
		radius = 0;
		return;
	}
	center = TilePos(accCenter.x / count, accCenter.y / count, accCenter.z / count);
	int_t maxRadiusSqr = 0;
	for (auto &d : doorInfos)
		maxRadiusSqr = std::max(d->distanceToSqr(center.x, center.y, center.z), maxRadiusSqr);
	radius = std::max(32, static_cast<int_t>(std::sqrt(static_cast<double>(maxRadiusSqr))) + 1);
}

int_t Village::getStanding(const jstring &player) const
{
	auto it = playerStanding.find(player);
	return it != playerStanding.end() ? it->second : 0;
}

int_t Village::modifyStanding(const jstring &player, int_t amount)
{
	int_t current = getStanding(player);
	int_t result = Mth::clamp(current + amount, -30, 10);
	playerStanding[player] = result;
	return result;
}

void Village::rewardAllPlayers(int_t amount)
{
	std::vector<jstring> names;
	for (const auto &e : playerStanding)
		names.push_back(e.first);
	for (const auto &name : names)
		modifyStanding(name, amount);
}

void Village::readAdditionalSaveData(CompoundTag &tag)
{
	populationSize = tag.getInt(u"PopSize");
	radius = tag.getInt(u"Radius");
	golemCount = tag.getInt(u"Golems");
	stableSince = tag.getInt(u"Stable");
	tick_ = tag.getInt(u"Tick");
	noBreedTimer = tag.getInt(u"MTick");
	center = TilePos(tag.getInt(u"CX"), tag.getInt(u"CY"), tag.getInt(u"CZ"));
	accCenter = TilePos(tag.getInt(u"ACX"), tag.getInt(u"ACY"), tag.getInt(u"ACZ"));
	if (std::shared_ptr<ListTag> doors = tag.getList(u"Doors"))
	{
		for (int_t i = 0; i < doors->size(); ++i)
		{
			auto d = std::static_pointer_cast<CompoundTag>(doors->get(i));
			doorInfos.push_back(std::make_unique<DoorInfo>(d->getInt(u"X"), d->getInt(u"Y"), d->getInt(u"Z"), d->getInt(u"IDX"), d->getInt(u"IDZ"), d->getInt(u"TS")));
		}
	}
	if (std::shared_ptr<ListTag> players = tag.getList(u"Players"))
	{
		for (int_t i = 0; i < players->size(); ++i)
		{
			auto p = std::static_pointer_cast<CompoundTag>(players->get(i));
			playerStanding[p->getString(u"Name")] = p->getInt(u"S");
		}
	}
}

void Village::addAdditonalSaveData(CompoundTag &tag)
{
	tag.putInt(u"PopSize", populationSize);
	tag.putInt(u"Radius", radius);
	tag.putInt(u"Golems", golemCount);
	tag.putInt(u"Stable", stableSince);
	tag.putInt(u"Tick", tick_);
	tag.putInt(u"MTick", noBreedTimer);
	tag.putInt(u"CX", center.x);
	tag.putInt(u"CY", center.y);
	tag.putInt(u"CZ", center.z);
	tag.putInt(u"ACX", accCenter.x);
	tag.putInt(u"ACY", accCenter.y);
	tag.putInt(u"ACZ", accCenter.z);
	auto doors = Util::make_shared<ListTag>();
	for (auto &d : doorInfos)
	{
		auto t = Util::make_shared<CompoundTag>();
		t->putInt(u"X", d->x);
		t->putInt(u"Y", d->y);
		t->putInt(u"Z", d->z);
		t->putInt(u"IDX", d->insideDx);
		t->putInt(u"IDZ", d->insideDz);
		t->putInt(u"TS", d->timeStamp);
		doors->add(t);
	}
	tag.put(u"Doors", doors);
	auto players = Util::make_shared<ListTag>();
	for (const auto &e : playerStanding)
	{
		auto t = Util::make_shared<CompoundTag>();
		t->putString(u"Name", e.first);
		t->putInt(u"S", e.second);
		players->add(t);
	}
	tag.put(u"Players", players);
}

// ---- Villages ----

Villages::Villages(Level &level) : MapDataBase(u"villages"), level(&level)
{
	markDirty();
}

void Villages::setLevel(Level &value)
{
	level = &value;
	for (auto &v : villages)
		v->setLevel(value);
}

void Villages::queryUpdateAround(int_t x, int_t y, int_t z)
{
	if (queries.size() > 64)
		return;
	if (!hasQuery(x, y, z))
		queries.emplace_back(x, y, z);
}

void Villages::tick()
{
	tick_++;
	for (auto &v : villages)
		v->tick(tick_);
	removeVillages();
	processNextQuery();
	cluster();
	if (tick_ % 400 == 0)
		markDirty();
}

void Villages::removeVillages()
{
	for (auto it = villages.begin(); it != villages.end();)
	{
		if ((*it)->canRemove())
		{
			it = villages.erase(it);
			markDirty();
		}
		else
			++it;
	}
}

Village *Villages::getClosestVillage(int_t x, int_t y, int_t z, int_t maxDist)
{
	Village *closest = nullptr;
	float closestDist = std::numeric_limits<float>::max();
	for (auto &v : villages)
	{
		float dist = tilePosDistSqr(v->getCenter(), x, y, z);
		if (dist >= closestDist)
			continue;
		int_t d = maxDist + v->getRadius();
		if (dist > static_cast<float>(d * d))
			continue;
		closest = v.get();
		closestDist = dist;
	}
	return closest;
}

void Villages::processNextQuery()
{
	if (queries.empty())
		return;
	TilePos pos = queries.front();
	queries.erase(queries.begin());
	addDoorInfos(pos);
}

void Villages::cluster()
{
	for (auto &door : unclustered)
	{
		bool found = false;
		for (auto &v : villages)
		{
			int_t dist = static_cast<int_t>(tilePosDistSqr(v->getCenter(), door->x, door->y, door->z));
			int_t radius = 32 + v->getRadius();
			if (dist <= radius * radius)
			{
				v->addDoorInfo(std::move(door));
				found = true;
				break;
			}
		}
		if (!found)
		{
			auto village = std::make_unique<Village>(*level);
			village->addDoorInfo(std::move(door));
			villages.push_back(std::move(village));
			markDirty();
		}
	}
	unclustered.clear();
}

void Villages::addDoorInfos(const TilePos &pos)
{
	const int_t sx = 16, sy = 4, sz = 16;
	for (int_t xx = pos.x - sx; xx < pos.x + sx; ++xx)
		for (int_t yy = pos.y - sy; yy < pos.y + sy; ++yy)
			for (int_t zz = pos.z - sz; zz < pos.z + sz; ++zz)
			{
				if (!isDoor(xx, yy, zz))
					continue;
				DoorInfo *door = getDoorInfo(xx, yy, zz);
				if (door == nullptr)
					createDoorInfo(xx, yy, zz);
				else
					door->timeStamp = tick_;
			}
}

DoorInfo *Villages::getDoorInfo(int_t x, int_t y, int_t z)
{
	for (auto &d : unclustered)
		if (d->x == x && d->z == z && std::abs(d->y - y) <= 1)
			return d.get();
	for (auto &v : villages)
		if (DoorInfo *d = v->getDoorInfo(x, y, z))
			return d;
	return nullptr;
}

void Villages::createDoorInfo(int_t x, int_t y, int_t z)
{
	int_t dir = level->getData(x, y, z) & 3;
	if (dir == 0 || dir == 2)
	{
		int_t canSee = 0;
		for (int_t i = -5; i < 0; ++i)
			if (level->canSeeSky(x + i, y, z))
				canSee--;
		for (int_t i = 1; i <= 5; ++i)
			if (level->canSeeSky(x + i, y, z))
				canSee++;
		if (canSee != 0)
			unclustered.push_back(std::make_unique<DoorInfo>(x, y, z, canSee > 0 ? -2 : 2, 0, tick_));
	}
	else
	{
		int_t canSee = 0;
		for (int_t i = -5; i < 0; ++i)
			if (level->canSeeSky(x, y, z + i))
				canSee--;
		for (int_t i = 1; i <= 5; ++i)
			if (level->canSeeSky(x, y, z + i))
				canSee++;
		if (canSee != 0)
			unclustered.push_back(std::make_unique<DoorInfo>(x, y, z, 0, canSee > 0 ? -2 : 2, tick_));
	}
}

bool Villages::hasQuery(int_t x, int_t y, int_t z) const
{
	for (const auto &q : queries)
		if (q.x == x && q.y == y && q.z == z)
			return true;
	return false;
}

bool Villages::isDoor(int_t x, int_t y, int_t z)
{
	return level->getTile(x, y, z) == Tile::doorWood.id;
}

void Villages::readFromNBT(CompoundTag &tag)
{
	tick_ = tag.getInt(u"Tick");
	if (std::shared_ptr<ListTag> list = tag.getList(u"Villages"))
	{
		for (int_t i = 0; i < list->size(); ++i)
		{
			auto v = std::make_unique<Village>();
			v->readAdditionalSaveData(*std::static_pointer_cast<CompoundTag>(list->get(i)));
			villages.push_back(std::move(v));
		}
	}
}

void Villages::writeToNBT(CompoundTag &tag)
{
	tag.putInt(u"Tick", tick_);
	auto list = Util::make_shared<ListTag>();
	for (auto &v : villages)
	{
		auto t = Util::make_shared<CompoundTag>();
		v->addAdditonalSaveData(*t);
		list->add(t);
	}
	tag.put(u"Villages", list);
}
