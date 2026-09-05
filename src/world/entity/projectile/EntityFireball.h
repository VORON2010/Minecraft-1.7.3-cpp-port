#pragma once

#include "world/entity/Entity.h"

class Mob;

class EntityFireball : public Entity
{
private:
	int_t xTile = -1;
	int_t yTile = -1;
	int_t zTile = -1;
	int_t inTile = 0;
	bool inGround = false;
	int_t ticksAlive = 0;
	int_t ticksInAir = 0;

public:
	int_t shakeTime = 0;
	std::weak_ptr<Entity> owner;
	double accelX = 0.0;
	double accelY = 0.0;
	double accelZ = 0.0;

	EntityFireball(Level &level);
	EntityFireball(Level &level, double x, double y, double z,
		double xPower, double yPower, double zPower);
	EntityFireball(Level &level, Mob &owner, double xPower, double yPower, double zPower);
	bool shouldRenderAtSqrDistance(double distance) override;
	void tick() override;
	bool isPickable() override;
	float getPickRadius() override;
	bool hurt(Entity *source, int_t dmg) override;
	jstring getEncodeId() const override { return u""; }

protected:
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
};
