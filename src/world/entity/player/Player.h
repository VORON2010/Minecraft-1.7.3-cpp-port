#pragma once

#include "world/entity/Mob.h"
#include "world/entity/player/InventoryPlayer.h"
#include "world/level/TilePos.h"
#include "world/level/tile/Tile.h"

#include "java/Type.h"
#include "java/String.h"

class EntityItem;
class EntityFish;
class StatBase;
class Container;

class Player : public Mob
{
public:
	static constexpr int_t MAX_HEALTH = 20;
	static constexpr int_t SWING_DURATION = 8;

	byte_t userType = 0;
	int_t score = 0;

	float oBob = 0.0f;
	float bob = 0.0f;

	bool swinging = false;
	int_t swingTime = 0;

	jstring name;
	int_t dimension = 0;

	jstring cloakTexture;
	double xCloakO = 0.0, yCloakO = 0.0, zCloakO = 0.0;
	double xCloak = 0.0, yCloak = 0.0, zCloak = 0.0;

	int_t dmgSpill = 0;
	InventoryPlayer inventory = InventoryPlayer(this);
	std::shared_ptr<Container> inventorySlots;
	std::shared_ptr<Container> craftingInventory;
	std::unique_ptr<TilePos> playerSpawnPosition;
	std::shared_ptr<EntityFish> fishEntity;

private:
	bool hasMinecartStart = false;
	int_t minecartStartX = 0;
	int_t minecartStartY = 0;
	int_t minecartStartZ = 0;


public:
	Player(Level &level);

	void tick() override;
	void prepareCustomTextures() override;
	virtual void closeContainer();

protected:
	virtual void resetHeight();

public:
	virtual void rideTick() override;
	virtual void resetPos() override;

protected:
	virtual void updateAi() override;
	void jumpFromGround() override;
	void causeFallDamage(float distance) override;

public:
	virtual void aiStep() override;
	void travel(float x, float z) override;

private:
	void touch(Entity &e);

public:
	virtual void swing();
	virtual void attack(const std::shared_ptr<Entity> &entity);
	virtual void respawn();
	void triggerAchievement(const StatBase &stat);
	virtual void addStat(const StatBase &stat, int_t amount);

	float getDestroySpeed(Tile &tile);
	bool canDestroy(Tile &tile);

	void readAdditionalSaveData(CompoundTag &tag) override;
	void addAdditionalSaveData(CompoundTag &tag) override;
	const TilePos *getPlayerSpawnPosition() const;
	void setPlayerSpawnPosition(const TilePos *position);

	float getHeadHeight() override;
	double getRidingHeight() override;
	void die(Entity *source) override;
	bool hurt(Entity *source, int_t dmg) override;

protected:
	void actuallyHurt(int_t dmg) override;

public:
	void interact(const std::shared_ptr<Entity> &entity);
	virtual void take(Entity &entity, int_t count);
	ItemInstance *getSelectedItem();
	void removeSelectedItem();
	virtual void drop();
	void drop(ItemInstance &stack);
	void drop(ItemInstance &stack, bool randomSpread);

protected:
	virtual void reallyDrop(std::shared_ptr<EntityItem> itemEntity);

public:
	bool sleeping = false;
	int_t sleepTimer = 0;
	int_t bedX = 0, bedY = 0, bedZ = 0;

	float bedViewX = 0.0f;
	float bedViewY = 0.0f;
	float bedViewZ = 0.0f;

	enum class SleepStatus
	{
		OK,
		NOT_POSSIBLE_HERE,
		NOT_POSSIBLE_NOW,
		TOO_FAR_AWAY,
		OTHER_PROBLEM
	};

	SleepStatus sleepInBedAt(int_t x, int_t y, int_t z);
	void wakeUpPlayer(bool immediately, bool updateSleepFlag, bool setSpawn);
	bool isPlayerSleeping() const { return sleeping; }
	bool isPlayerFullyAsleep() const { return sleeping && sleepTimer >= 100; }
	bool isInBed() const;
	float getBedOrientationInDegrees() const;

	virtual void displayClientMessage(const jstring &message) {}
	virtual void sendChatMessage(const jstring &message) { (void)message; }
	virtual void animateRespawn() {}
	void awardKillScore(Entity &source, int_t score) override;

public:
	bool isPlayer() override { return true; }
};
