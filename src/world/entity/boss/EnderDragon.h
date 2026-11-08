#pragma once

#include <array>
#include <memory>
#include <vector>

#include "world/entity/Mob.h"

class EnderDragon;
class EnderCrystal;

// 1.0 EnderDragonPart: a hitbox that forwards damage to the dragon
class EnderDragonPart : public Entity
{
public:
	EnderDragon &dragon;
	const jstring id;

	EnderDragonPart(EnderDragon &dragon, const jstring &id, float w, float h);
	jstring getEncodeId() const override { return u""; }
	bool isPickable() override { return true; }
	bool hurt(Entity *source, int_t dmg) override;
	bool is(const Entity &other) const override;

protected:
	void addAdditionalSaveData(CompoundTag &tag) override { (void)tag; }
	void readAdditionalSaveData(CompoundTag &tag) override { (void)tag; }
};

// 1.0 EnderDragonEntity
class EnderDragon : public Mob
{
public:
	static constexpr int_t MAX_HEALTH = 200;

	double xTarget = 0.0, yTarget = 100.0, zTarget = 0.0;
	std::array<std::array<double, 3>, 64> positions = {};
	int_t posPointer = -1;
	std::shared_ptr<EnderDragonPart> head, body, tail1, tail2, tail3, wing1, wing2;
	std::vector<std::shared_ptr<Entity>> subEntities;
	float oFlapTime = 0.0f;
	float flapTime = 0.0f;
	bool newTarget = false;
	bool inWall = false;
	int_t dragonDeathTime = 0;
	std::shared_ptr<EnderCrystal> nearestCrystal;

private:
	std::shared_ptr<Entity> attackTarget;

public:
	EnderDragon(Level &level);
	jstring getEncodeId() const override { return u"EnderDragon"; }

	std::array<double, 3> getLatencyPos(int_t step, float a);
	void aiStep() override;
	bool hurt(Entity *source, int_t dmg) override;
	bool hurtPart(EnderDragonPart &part, Entity *source, int_t dmg, bool explosion = false);
	bool isPickable() override { return false; }
	const std::vector<std::shared_ptr<Entity>> &getSubEntities() const override { return subEntities; }
	int_t getSynchedHealth();

protected:
	void tickDeath() override;
	jstring getAmbientSound() override;
	jstring getHurtSound() override;
	float getSoundVolume() override;
	void checkDespawn() override {}

private:
	void checkCrystals();
	void knockBack(const std::vector<std::shared_ptr<Entity>> &entities);
	void hurtEntities(const std::vector<std::shared_ptr<Entity>> &entities);
	void findNewTarget();
	float rotWrap(double angle);
	bool checkWalls(const AABB &box);
	void spawnExitPortal(int_t x, int_t z);
};
