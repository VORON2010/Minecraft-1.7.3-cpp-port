#include "world/entity/ai/Goals.h"

#include <limits>

#include "world/entity/ai/Controls.h"
#include "world/entity/ai/PathNavigation.h"
#include "world/entity/ai/RandomPos.h"
#include "world/entity/PathfinderMob.h"
#include "world/entity/npc/Villager.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/tile/DoorTile.h"
#include "world/level/tile/Tile.h"
#include "world/phys/AABB.h"
#include "world/village/Village.h"
#include "util/Mth.h"

std::shared_ptr<Entity> getClosestEntityOfClass(Mob &mob, const EntityTypeTest &type, double xz, double y)
{
	AABB *box = mob.bb.grow(xz, y, xz);
	std::shared_ptr<Entity> best;
	double bestDist = std::numeric_limits<double>::max();
	for (const auto &e : mob.level.getEntities(nullptr, *box))
	{
		if (e.get() == &mob || !type(*e))
			continue;
		double d = mob.distanceToSqr(*e);
		if (d <= bestDist)
		{
			best = e;
			bestDist = d;
		}
	}
	return best;
}

// ---- FloatGoal ----

FloatGoal::FloatGoal(Mob &mob) : mob(mob)
{
	setFlags(4);
	mob.getNavigation().setCanFloat(true);
}

bool FloatGoal::canUse()
{
	return mob.isInWater() || mob.isInLava();
}

void FloatGoal::tick()
{
	if (mob.getRandom().nextFloat() < 0.8f)
		mob.getJumpControl().jumpOnce();
}

// ---- AvoidEntityGoal ----

AvoidEntityGoal::AvoidEntityGoal(PathfinderMob &mob, EntityTypeTest avoidType, bool avoidPlayers, float maxDist, float walkSpeed, float sprintSpeed)
	: mob(mob), walkSpeed(walkSpeed), sprintSpeed(sprintSpeed), maxDist(maxDist), avoidType(std::move(avoidType)), avoidPlayers(avoidPlayers)
{
	setFlags(1);
}

bool AvoidEntityGoal::canUse()
{
	if (avoidPlayers)
	{
		toAvoid = mob.level.getNearestPlayer(mob, maxDist);
		if (toAvoid == nullptr)
			return false;
	}
	else
	{
		AABB *box = mob.bb.grow(maxDist, 3.0, maxDist);
		toAvoid = nullptr;
		for (const auto &e : mob.level.getEntities(nullptr, *box))
		{
			if (e.get() != &mob && avoidType(*e) && e->isAlive() && mob.canSeeCached(*e))
			{
				toAvoid = e;
				break;
			}
		}
		if (toAvoid == nullptr)
			return false;
	}

	Vec3 posv(0.0, 0.0, 0.0);
	Vec3 *pos = &posv;
	if (!RandomPos::getPosAvoid(mob, 16, 7, Vec3(toAvoid->x, toAvoid->y, toAvoid->z), posv))
		return false;
	if (toAvoid->distanceToSqr(pos->x, pos->y, pos->z) < toAvoid->distanceToSqr(mob))
		return false;
	path = mob.getNavigation().createPath(pos->x, pos->y, pos->z);
	return path != nullptr && path->endsInXZ(*pos);
}

bool AvoidEntityGoal::canContinueToUse()
{
	return !mob.getNavigation().isDone();
}

void AvoidEntityGoal::start()
{
	mob.getNavigation().moveTo(path, walkSpeed);
}

void AvoidEntityGoal::stop()
{
	toAvoid = nullptr;
}

void AvoidEntityGoal::tick()
{
	if (mob.distanceToSqr(*toAvoid) < 49.0)
		mob.getNavigation().setSpeed(sprintSpeed);
	else
		mob.getNavigation().setSpeed(walkSpeed);
}

// ---- TradeWithPlayerGoal ----

TradeWithPlayerGoal::TradeWithPlayerGoal(Villager &mob) : mob(mob)
{
	setFlags(5);
}

bool TradeWithPlayerGoal::canUse()
{
	if (!mob.isAlive() || mob.isInWater() || !mob.onGround || mob.hurtMarked)
		return false;
	Player *trader = mob.getTradingPlayer();
	if (trader == nullptr)
		return false;
	return mob.distanceToSqr(*trader) <= 16.0;
}

void TradeWithPlayerGoal::start()
{
	mob.getNavigation().stop();
}

void TradeWithPlayerGoal::stop()
{
	mob.setTradingPlayer(nullptr);
}

// ---- LookAtPlayerGoal ----

LookAtPlayerGoal::LookAtPlayerGoal(Mob &mob, EntityTypeTest type, bool players, float lookDistance, float probability)
	: mob(mob), lookDistance(lookDistance), probability(probability), lookAtType(std::move(type)), players(players)
{
	setFlags(2);
}

bool LookAtPlayerGoal::canUse()
{
	if (mob.getRandom().nextFloat() >= probability)
		return false;
	if (players)
		lookAt = mob.level.getNearestPlayer(mob, lookDistance);
	else
		lookAt = getClosestEntityOfClass(mob, lookAtType, lookDistance, 3.0);
	return lookAt != nullptr;
}

bool LookAtPlayerGoal::canContinueToUse()
{
	if (!lookAt->isAlive())
		return false;
	if (mob.distanceToSqr(*lookAt) > lookDistance * lookDistance)
		return false;
	return lookTime > 0;
}

void LookAtPlayerGoal::start()
{
	lookTime = 40 + mob.getRandom().nextInt(40);
}

void LookAtPlayerGoal::stop()
{
	lookAt = nullptr;
}

void LookAtPlayerGoal::tick()
{
	mob.getLookControl().setLookAt(lookAt->x, lookAt->y + lookAt->getHeadHeight(), lookAt->z, 10.0f, static_cast<float>(mob.getMaxHeadXRot()));
	lookTime--;
}

InteractGoal::InteractGoal(Mob &mob, EntityTypeTest type, bool players, float lookDistance, float probability)
	: LookAtPlayerGoal(mob, std::move(type), players, lookDistance, probability)
{
	setFlags(3);
}

// ---- LookAtTradingPlayerGoal ----

LookAtTradingPlayerGoal::LookAtTradingPlayerGoal(Villager &villager)
	: LookAtPlayerGoal(villager, [](Entity &) { return true; }, true, 8.0f), villager(villager)
{
}

bool LookAtTradingPlayerGoal::canUse()
{
	if (!villager.isTrading())
		return false;
	lookAt = villager.level.getEntityRef(*villager.getTradingPlayer());
	return true;
}

// ---- MoveIndoorsGoal ----

MoveIndoorsGoal::MoveIndoorsGoal(PathfinderMob &mob) : mob(mob)
{
	setFlags(1);
}

bool MoveIndoorsGoal::canUse()
{
	Level &level = mob.level;
	if ((level.isDay() && !level.isRaining()) || level.dimension->hasCeiling)
		return false;
	if (mob.getRandom().nextInt(50) != 0)
		return false;
	if (insideX != -1 && mob.distanceToSqr(insideX, mob.y, insideZ) < 4.0)
		return false;
	Village *village = level.getVillages().getClosestVillage(Mth::floor(mob.x), Mth::floor(mob.y), Mth::floor(mob.z), 14);
	if (village == nullptr)
		return false;
	doorInfo = village->getBestDoorInfo(Mth::floor(mob.x), Mth::floor(mob.y), Mth::floor(mob.z));
	return doorInfo != nullptr;
}

bool MoveIndoorsGoal::canContinueToUse()
{
	return !mob.getNavigation().isDone();
}

void MoveIndoorsGoal::start()
{
	insideX = -1;
	if (mob.distanceToSqr(doorInfo->getIndoorX(), doorInfo->y, doorInfo->getIndoorZ()) > 256.0)
	{
		Vec3 pos(0.0, 0.0, 0.0);
		if (RandomPos::getPosTowards(mob, 14, 3, Vec3(doorInfo->getIndoorX() + 0.5, doorInfo->getIndoorY(), doorInfo->getIndoorZ() + 0.5), pos))
			mob.getNavigation().moveTo(pos.x, pos.y, pos.z, 0.3f);
	}
	else
		mob.getNavigation().moveTo(doorInfo->getIndoorX() + 0.5, doorInfo->getIndoorY(), doorInfo->getIndoorZ() + 0.5, 0.3f);
}

void MoveIndoorsGoal::stop()
{
	insideX = doorInfo->getIndoorX();
	insideZ = doorInfo->getIndoorZ();
	doorInfo = nullptr;
}

// ---- RestrictOpenDoorGoal ----

RestrictOpenDoorGoal::RestrictOpenDoorGoal(PathfinderMob &mob) : mob(mob)
{
}

bool RestrictOpenDoorGoal::canUse()
{
	if (mob.level.isDay())
		return false;
	Village *village = mob.level.getVillages().getClosestVillage(Mth::floor(mob.x), Mth::floor(mob.y), Mth::floor(mob.z), 16);
	if (village == nullptr)
		return false;
	doorInfo = village->getClosestDoorInfo(Mth::floor(mob.x), Mth::floor(mob.y), Mth::floor(mob.z));
	if (doorInfo == nullptr)
		return false;
	return doorInfo->distanceToInsideSqr(Mth::floor(mob.x), Mth::floor(mob.y), Mth::floor(mob.z)) < 2.25;
}

bool RestrictOpenDoorGoal::canContinueToUse()
{
	if (mob.level.isDay())
		return false;
	return !doorInfo->removed && doorInfo->isInsideSide(Mth::floor(mob.x), Mth::floor(mob.z));
}

void RestrictOpenDoorGoal::start()
{
	mob.getNavigation().setCanOpenDoors(false);
	mob.getNavigation().setCanPassDoors(false);
}

void RestrictOpenDoorGoal::stop()
{
	mob.getNavigation().setCanOpenDoors(true);
	mob.getNavigation().setCanPassDoors(true);
	doorInfo = nullptr;
}

void RestrictOpenDoorGoal::tick()
{
	doorInfo->incBookingCount();
}

// ---- OpenDoorGoal ----

OpenDoorGoal::OpenDoorGoal(Mob &mob, bool closeDoorAfter) : mob(mob), closeDoor(closeDoorAfter)
{
}

DoorTile *OpenDoorGoal::getDoorTile(int_t x, int_t y, int_t z)
{
	int_t tile = mob.level.getTile(x, y, z);
	if (tile != Tile::doorWood.id)
		return nullptr;
	return &Tile::doorWood;
}

bool OpenDoorGoal::canUse()
{
	if (!mob.horizontalCollision)
		return false;
	PathNavigation &nav = mob.getNavigation();
	NavPath *path = nav.getPath();
	if (path == nullptr || path->isDone() || !nav.getCanOpenDoors())
		return false;
	for (int_t i = 0; i < std::min(path->getIndex() + 2, path->getSize()); ++i)
	{
		Node *node = path->get(i);
		doorX = node->x;
		doorY = node->y + 1;
		doorZ = node->z;
		if (mob.distanceToSqr(doorX, mob.y, doorZ) > 2.25)
			continue;
		doorTile = getDoorTile(doorX, doorY, doorZ);
		if (doorTile != nullptr)
			return true;
	}
	doorX = Mth::floor(mob.x);
	doorY = Mth::floor(mob.y + 1.0);
	doorZ = Mth::floor(mob.z);
	doorTile = getDoorTile(doorX, doorY, doorZ);
	return doorTile != nullptr;
}

bool OpenDoorGoal::canContinueToUse()
{
	return closeDoor && forgetTime > 0 && !passed;
}

void OpenDoorGoal::start()
{
	passed = false;
	doorOpenDirX = static_cast<float>(doorX + 0.5f - mob.x);
	doorOpenDirZ = static_cast<float>(doorZ + 0.5f - mob.z);
	forgetTime = 20;
	doorTile->setOpen(mob.level, doorX, doorY, doorZ, true);
}

void OpenDoorGoal::stop()
{
	if (closeDoor)
		doorTile->setOpen(mob.level, doorX, doorY, doorZ, false);
}

void OpenDoorGoal::tick()
{
	forgetTime--;
	float dx = static_cast<float>(doorX + 0.5f - mob.x);
	float dz = static_cast<float>(doorZ + 0.5f - mob.z);
	if (doorOpenDirX * dx + doorOpenDirZ * dz < 0.0f)
		passed = true;
}

// ---- MoveTowardsRestrictionGoal ----

MoveTowardsRestrictionGoal::MoveTowardsRestrictionGoal(PathfinderMob &mob, float speed) : mob(mob), speed(speed)
{
	setFlags(1);
}

bool MoveTowardsRestrictionGoal::canUse()
{
	if (mob.isWithinRestriction())
		return false;
	Vec3 pos(0.0, 0.0, 0.0);
	if (!RandomPos::getPosTowards(mob, 16, 7, Vec3(mob.getRestrictX(), mob.getRestrictY(), mob.getRestrictZ()), pos))
		return false;
	wantedX = pos.x;
	wantedY = pos.y;
	wantedZ = pos.z;
	return true;
}

bool MoveTowardsRestrictionGoal::canContinueToUse()
{
	return !mob.getNavigation().isDone();
}

void MoveTowardsRestrictionGoal::start()
{
	mob.getNavigation().moveTo(wantedX, wantedY, wantedZ, speed);
}

// ---- MakeLoveGoal ----

MakeLoveGoal::MakeLoveGoal(Villager &villager) : villager(villager)
{
	setFlags(3);
}

bool MakeLoveGoal::canUse()
{
	if (villager.getAge() != 0)
		return false;
	if (villager.getRandom().nextInt(500) != 0)
		return false;
	village = villager.level.getVillages().getClosestVillage(Mth::floor(villager.x), Mth::floor(villager.y), Mth::floor(villager.z), 0);
	if (village == nullptr)
		return false;
	if (!villageNeedsMoreVillagers())
		return false;
	std::shared_ptr<Entity> mate = getClosestEntityOfClass(villager, [](Entity &e) { return dynamic_cast<Villager *>(&e) != nullptr; }, 8.0, 3.0);
	if (mate == nullptr)
		return false;
	partner = std::static_pointer_cast<Villager>(mate);
	return partner->getAge() == 0;
}

void MakeLoveGoal::start()
{
	loveMakingTime = 300;
	villager.setInLove(true);
}

void MakeLoveGoal::stop()
{
	village = nullptr;
	partner = nullptr;
	villager.setInLove(false);
}

bool MakeLoveGoal::canContinueToUse()
{
	return loveMakingTime >= 0 && villageNeedsMoreVillagers() && villager.getAge() == 0;
}

void MakeLoveGoal::tick()
{
	loveMakingTime--;
	villager.getLookControl().setLookAt(*partner, 10.0f, 30.0f);
	if (villager.distanceToSqr(*partner) > 2.25)
		villager.getNavigation().moveTo(*partner, 0.25f);
	else if (loveMakingTime == 0 && partner->isInLove())
		breed();
	if (villager.getRandom().nextInt(35) == 0)
		villager.sendEntityEvent(12);
}

bool MakeLoveGoal::villageNeedsMoreVillagers()
{
	if (!village->isBreedTimerOk())
		return false;
	int_t idealSize = static_cast<int_t>(village->getDoorCount() * 0.35);
	return village->getPopulationSize() < idealSize;
}

void MakeLoveGoal::breed()
{
	std::shared_ptr<Villager> child = villager.getBreedOffspringVillager(*partner);
	partner->setAge(6000);
	villager.setAge(6000);
	child->setAge(-24000);
	child->moveTo(villager.x, villager.y, villager.z, 0.0f, 0.0f);
	villager.level.addEntity(child);
	child->sendEntityEvent(12);
}

// ---- PlayGoal ----

PlayGoal::PlayGoal(Villager &mob, float speed) : mob(mob), speed(speed)
{
	setFlags(1);
}

bool PlayGoal::canUse()
{
	if (mob.getAge() >= 0)
		return false;
	if (mob.getRandom().nextInt(400) != 0)
		return false;
	AABB *box = mob.bb.grow(6.0, 3.0, 6.0);
	double closestDistSqr = std::numeric_limits<double>::max();
	for (const auto &e : mob.level.getEntities(nullptr, *box))
	{
		auto *v = dynamic_cast<Villager *>(e.get());
		if (v == nullptr || v == &mob || v->isChasing() || v->getAge() >= 0)
			continue;
		double d = v->distanceToSqr(mob);
		if (!(d > closestDistSqr))
		{
			closestDistSqr = d;
			friendMob = std::static_pointer_cast<Villager>(e);
		}
	}
	if (friendMob == nullptr)
	{
		Vec3 unused(0.0, 0.0, 0.0);
		if (!RandomPos::getPos(mob, 16, 3, unused))
			return false;
	}
	return true;
}

bool PlayGoal::canContinueToUse()
{
	return playTime > 0;
}

void PlayGoal::start()
{
	if (friendMob != nullptr)
		mob.setChasing(true);
	playTime = 1000;
}

void PlayGoal::stop()
{
	mob.setChasing(false);
	friendMob = nullptr;
}

void PlayGoal::tick()
{
	playTime--;
	if (friendMob != nullptr)
	{
		if (mob.distanceToSqr(*friendMob) > 4.0)
			mob.getNavigation().moveTo(*friendMob, speed);
	}
	else if (mob.getNavigation().isDone())
	{
		Vec3 pos(0.0, 0.0, 0.0);
		if (!RandomPos::getPos(mob, 16, 3, pos))
			return;
		mob.getNavigation().moveTo(pos.x, pos.y, pos.z, speed);
	}
}

// ---- RandomStrollGoal ----

RandomStrollGoal::RandomStrollGoal(PathfinderMob &mob, float speed) : mob(mob), speed(speed)
{
	setFlags(1);
}

bool RandomStrollGoal::canUse()
{
	if (mob.getNoActionTime() >= 100)
		return false;
	if (mob.getRandom().nextInt(120) != 0)
		return false;
	Vec3 pos(0.0, 0.0, 0.0);
	if (!RandomPos::getPos(mob, 10, 7, pos))
		return false;
	wantedX = pos.x;
	wantedY = pos.y;
	wantedZ = pos.z;
	return true;
}

bool RandomStrollGoal::canContinueToUse()
{
	return !mob.getNavigation().isDone();
}

void RandomStrollGoal::start()
{
	mob.getNavigation().moveTo(wantedX, wantedY, wantedZ, speed);
}
