#pragma once

#include "world/entity/Mob.h"

class EntityFireball;

class Ghast : public Mob
{
private:
	std::weak_ptr<Entity> target;
	int_t aggroCooldown = 0;
	double waypointX = 0.0;
	double waypointY = 0.0;
	double waypointZ = 0.0;
	int_t courseChangeCooldown = 0;
	bool isCourseTraversable(double x, double y, double z, double distance);

public:
	int_t prevAttackCounter = 0;
	int_t attackCounter = 0;

	Ghast(Level &level);
	jstring getEncodeId() const override { return u"Ghast"; }
	jstring getTexture() override;
	void tick() override;
	void updateAi() override;
	void travel(float x, float z) override;
	void causeFallDamage(float distance) override;
	bool canSpawn() override;
	int_t getMaxSpawnClusterSize() override;

protected:
	jstring getAmbientSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDeathLoot() override;
	float getSoundVolume() override;
};
