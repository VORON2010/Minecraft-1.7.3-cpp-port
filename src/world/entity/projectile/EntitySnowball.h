#pragma once

#include "world/entity/Entity.h"

class Mob;
class Player;

class EntitySnowball : public Entity
{
protected:
	int_t xTile = -1;
	int_t yTile = -1;
	int_t zTile = -1;
	int_t inTile = 0;
	bool inGround = false;
	std::weak_ptr<Entity> owner;
	int_t ticksInGround = 0;
	int_t ticksInAir = 0;

public:
	int_t shakeTime = 0;

	EntitySnowball(Level &level);
	EntitySnowball(Level &level, double x, double y, double z);
	EntitySnowball(Level &level, Mob &owner);

	jstring getEncodeId() const override { return u"Snowball"; }
	bool shouldRenderAtSqrDistance(double distance) override;
	void shoot(double xd, double yd, double zd, float speed, float spread);
	void setVelocity(double xd, double yd, double zd);
	void lerpMotion(double xd, double yd, double zd) override { setVelocity(xd, yd, zd); }
	void tick() override;
	float getShadowHeightOffs() override { return 0.0f; }

protected:
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
};
