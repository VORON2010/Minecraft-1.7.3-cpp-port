#pragma once

#include <functional>
#include <memory>

#include "world/entity/ai/Goal.h"
#include "world/phys/Vec3.h"

class Entity;
class Mob;
class PathfinderMob;
class Villager;
class NavPath;
class Village;
struct DoorInfo;
class DoorTile;

using EntityTypeTest = std::function<bool(Entity &)>;

// 1.5.2 SwimGoal (Mojang FloatGoal)
class FloatGoal : public Goal
{
	Mob &mob;

public:
	explicit FloatGoal(Mob &mob);
	bool canUse() override;
	void tick() override;
};

// 1.5.2 FleeEntityGoal (Mojang AvoidEntityGoal)
class AvoidEntityGoal : public Goal
{
	PathfinderMob &mob;
	float walkSpeed, sprintSpeed;
	std::shared_ptr<Entity> toAvoid;
	float maxDist;
	std::shared_ptr<NavPath> path;
	EntityTypeTest avoidType;
	bool avoidPlayers;

public:
	AvoidEntityGoal(PathfinderMob &mob, EntityTypeTest avoidType, bool avoidPlayers, float maxDist, float walkSpeed, float sprintSpeed);
	bool canUse() override;
	bool canContinueToUse() override;
	void start() override;
	void stop() override;
	void tick() override;
};

// 1.5.2 StopFollowingCustomerGoal (Mojang TradeWithPlayerGoal)
class TradeWithPlayerGoal : public Goal
{
	Villager &mob;

public:
	explicit TradeWithPlayerGoal(Villager &mob);
	bool canUse() override;
	void start() override;
	void stop() override;
};

// 1.5.2 LookAtEntityGoal (Mojang LookAtPlayerGoal)
class LookAtPlayerGoal : public Goal
{
protected:
	Mob &mob;
	std::shared_ptr<Entity> lookAt;
	float lookDistance;
	int_t lookTime = 0;
	float probability;
	EntityTypeTest lookAtType;
	bool players;

public:
	LookAtPlayerGoal(Mob &mob, EntityTypeTest type, bool players, float lookDistance, float probability = 0.02f);
	bool canUse() override;
	bool canContinueToUse() override;
	void start() override;
	void stop() override;
	void tick() override;
};

// 1.5.2 GoToEntityGoal (Mojang InteractGoal): a LookAtPlayerGoal that also blocks walking
class InteractGoal : public LookAtPlayerGoal
{
public:
	InteractGoal(Mob &mob, EntityTypeTest type, bool players, float lookDistance, float probability);
};

// 1.5.2 LookAtCustomerGoal (Mojang LookAtTradingPlayerGoal)
class LookAtTradingPlayerGoal : public LookAtPlayerGoal
{
	Villager &villager;

public:
	explicit LookAtTradingPlayerGoal(Villager &villager);
	bool canUse() override;
};

// 1.5.2 StayIndoorsGoal (Mojang MoveIndoorsGoal)
class MoveIndoorsGoal : public Goal
{
	PathfinderMob &mob;
	DoorInfo *doorInfo = nullptr;
	int_t insideX = -1;
	int_t insideZ = -1;

public:
	explicit MoveIndoorsGoal(PathfinderMob &mob);
	bool canUse() override;
	bool canContinueToUse() override;
	void start() override;
	void stop() override;
};

// 1.5.2 RestrictOpenDoorGoal
class RestrictOpenDoorGoal : public Goal
{
	PathfinderMob &mob;
	DoorInfo *doorInfo = nullptr;

public:
	explicit RestrictOpenDoorGoal(PathfinderMob &mob);
	bool canUse() override;
	bool canContinueToUse() override;
	void start() override;
	void stop() override;
	void tick() override;
};

// 1.5.2 AbstractDoorInteractGoal + LongDoorInteractGoal (Mojang OpenDoorGoal)
class OpenDoorGoal : public Goal
{
	Mob &mob;
	int_t doorX = 0, doorY = 0, doorZ = 0;
	DoorTile *doorTile = nullptr;
	bool passed = false;
	float doorOpenDirX = 0.0f, doorOpenDirZ = 0.0f;
	bool closeDoor;
	int_t forgetTime = 0;

	DoorTile *getDoorTile(int_t x, int_t y, int_t z);

public:
	OpenDoorGoal(Mob &mob, bool closeDoorAfter);
	bool canUse() override;
	bool canContinueToUse() override;
	void start() override;
	void stop() override;
	void tick() override;
};

// 1.5.2 WanderThroughVillageGoal (Mojang MoveTowardsRestrictionGoal)
class MoveTowardsRestrictionGoal : public Goal
{
	PathfinderMob &mob;
	double wantedX = 0.0, wantedY = 0.0, wantedZ = 0.0;
	float speed;

public:
	MoveTowardsRestrictionGoal(PathfinderMob &mob, float speed);
	bool canUse() override;
	bool canContinueToUse() override;
	void start() override;
};

// 1.5.2 VillagerMatingGoal (Mojang MakeLoveGoal)
class MakeLoveGoal : public Goal
{
	Villager &villager;
	std::shared_ptr<Villager> partner;
	int_t loveMakingTime = 0;
	Village *village = nullptr;

	bool villageNeedsMoreVillagers();
	void breed();

public:
	explicit MakeLoveGoal(Villager &villager);
	bool canUse() override;
	void start() override;
	void stop() override;
	bool canContinueToUse() override;
	void tick() override;
};

// 1.5.2 FormCaravanGoal (Mojang PlayGoal): children chase each other
class PlayGoal : public Goal
{
	Villager &mob;
	std::shared_ptr<Villager> friendMob;
	float speed;
	int_t playTime = 0;

public:
	PlayGoal(Villager &mob, float speed);
	bool canUse() override;
	bool canContinueToUse() override;
	void start() override;
	void stop() override;
	void tick() override;
};

// 1.5.2 WanderAroundGoal (Mojang RandomStrollGoal)
class RandomStrollGoal : public Goal
{
	PathfinderMob &mob;
	double wantedX = 0.0, wantedY = 0.0, wantedZ = 0.0;
	float speed;

public:
	RandomStrollGoal(PathfinderMob &mob, float speed);
	bool canUse() override;
	bool canContinueToUse() override;
	void start() override;
};

// 1.5.2 World.getNearestEntity(Class, box, entity)
std::shared_ptr<Entity> getClosestEntityOfClass(Mob &mob, const EntityTypeTest &type, double xz, double y);
