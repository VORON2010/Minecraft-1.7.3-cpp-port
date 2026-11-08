#pragma once

#include <map>
#include <memory>
#include <vector>

#include "java/String.h"
#include "java/Type.h"
#include "world/level/MapDataBase.h"
#include "world/level/TilePos.h"

class Level;
class Mob;
class Player;
class CompoundTag;

// 1.5.2 VillageDoor (Mojang DoorInfo): a wooden door with sky on one side
struct DoorInfo
{
	int_t x, y, z;
	int_t insideDx, insideDz;
	int_t timeStamp;
	bool removed = false;
	int_t bookings = 0;

	DoorInfo(int_t x, int_t y, int_t z, int_t insideDx, int_t insideDz, int_t timeStamp)
		: x(x), y(y), z(z), insideDx(insideDx), insideDz(insideDz), timeStamp(timeStamp) {}

	int_t distanceToSqr(int_t xx, int_t yy, int_t zz) const;
	int_t distanceToInsideSqr(int_t xx, int_t yy, int_t zz) const;
	int_t getIndoorX() const { return x + insideDx; }
	int_t getIndoorY() const { return y; }
	int_t getIndoorZ() const { return z + insideDz; }
	bool isInsideSide(int_t xx, int_t zz) const;
	void resetBookingCount() { bookings = 0; }
	void incBookingCount() { bookings++; }
	int_t getBookingsCount() const { return bookings; }
};

// 1.5.2 Village
class Village
{
	struct Aggressor
	{
		std::weak_ptr<Mob> mob;
		int_t timeStamp;
	};

	Level *level = nullptr;
	std::vector<std::unique_ptr<DoorInfo>> doorInfos;
	TilePos accCenter{0, 0, 0};
	TilePos center{0, 0, 0};
	int_t radius = 0;
	int_t stableSince = 0;
	int_t tick_ = 0;
	int_t populationSize = 0;
	int_t noBreedTimer = 0;
	std::map<jstring, int_t> playerStanding;
	std::vector<Aggressor> aggressors;
	int_t golemCount = 0;

	bool canSpawnAt(int_t x, int_t y, int_t z, int_t sx, int_t sy, int_t sz);
	void countGolem();
	void countPopulation();
	void updateAggressors();
	void updateDoors();
	bool isDoor(int_t x, int_t y, int_t z);
	void calcInfo();

public:
	Village() {}
	explicit Village(Level &level) : level(&level) {}

	void setLevel(Level &value) { level = &value; }
	void tick(int_t tick);
	const TilePos &getCenter() const { return center; }
	int_t getRadius() const { return radius; }
	int_t getDoorCount() const { return static_cast<int_t>(doorInfos.size()); }
	int_t getStableAge() const { return tick_ - stableSince; }
	int_t getPopulationSize() const { return populationSize; }
	bool isInside(int_t x, int_t y, int_t z) const;
	const std::vector<std::unique_ptr<DoorInfo>> &getDoorInfos() const { return doorInfos; }
	DoorInfo *getClosestDoorInfo(int_t x, int_t y, int_t z);
	DoorInfo *getBestDoorInfo(int_t x, int_t y, int_t z);
	DoorInfo *getDoorInfo(int_t x, int_t y, int_t z);
	void addDoorInfo(std::unique_ptr<DoorInfo> door);
	bool canRemove() const { return doorInfos.empty(); }
	void addAggressor(const std::shared_ptr<Mob> &mob);
	std::shared_ptr<Mob> getClosestAggressor(Mob &from);
	std::shared_ptr<Player> getClosestBadStandingPlayer(Mob &from);
	int_t getStanding(const jstring &player) const;
	int_t modifyStanding(const jstring &player, int_t amount);
	bool isVeryBadStanding(const jstring &player) const { return getStanding(player) <= -15; }
	void readAdditionalSaveData(CompoundTag &tag);
	void addAdditonalSaveData(CompoundTag &tag);
	void resetNoBreedTimer() { noBreedTimer = tick_; }
	bool isBreedTimerOk() const { return noBreedTimer == 0 || tick_ - noBreedTimer >= 3600; }
	void rewardAllPlayers(int_t amount);
};

// 1.5.2 SavedVillageData (Mojang Villages): finds doors near villagers and groups them into villages
class Villages : public MapDataBase
{
	Level *level = nullptr;
	std::vector<TilePos> queries;
	std::vector<std::unique_ptr<DoorInfo>> unclustered;
	std::vector<std::unique_ptr<Village>> villages;
	int_t tick_ = 0;

	void removeVillages();
	void processNextQuery();
	void cluster();
	void addDoorInfos(const TilePos &pos);
	DoorInfo *getDoorInfo(int_t x, int_t y, int_t z);
	void createDoorInfo(int_t x, int_t y, int_t z);
	bool hasQuery(int_t x, int_t y, int_t z) const;
	bool isDoor(int_t x, int_t y, int_t z);

public:
	explicit Villages(const jstring &id) : MapDataBase(id) {}
	explicit Villages(Level &level);

	void setLevel(Level &value);
	void queryUpdateAround(int_t x, int_t y, int_t z);
	void tick();
	const std::vector<std::unique_ptr<Village>> &getVillages() const { return villages; }
	Village *getClosestVillage(int_t x, int_t y, int_t z, int_t maxDist);

	void readFromNBT(CompoundTag &tag) override;
	void writeToNBT(CompoundTag &tag) override;
};

inline float tilePosDistSqr(const TilePos &p, int_t x, int_t y, int_t z)
{
	float dx = static_cast<float>(p.x - x), dy = static_cast<float>(p.y - y), dz = static_cast<float>(p.z - z);
	return dx * dx + dy * dy + dz * dz;
}
