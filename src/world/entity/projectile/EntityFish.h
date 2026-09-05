#pragma once

#include <memory>

#include "world/entity/Entity.h"

class Player;

class EntityFish : public Entity
{
private:
	int_t xTile = -1;
	int_t yTile = -1;
	int_t zTile = -1;
	int_t inTile = 0;
	bool inGround = false;
	int_t ticksInGround = 0;
	int_t ticksInAir = 0;
	int_t ticksCatchable = 0;
	int_t lerpSteps = 0;
	double lerpX = 0.0;
	double lerpY = 0.0;
	double lerpZ = 0.0;
	double lerpYRot = 0.0;
	double lerpXRot = 0.0;
	double velocityX = 0.0;
	double velocityY = 0.0;
	double velocityZ = 0.0;

public:
	int_t shake = 0;
	Player *angler = nullptr;
	std::shared_ptr<Entity> bobber;

	explicit EntityFish(Level &level);
	EntityFish(Level &level, double x, double y, double z);
	EntityFish(Level &level, Player &angler);

	bool shouldRenderAtSqrDistance(double distance) override;
	void shoot(double xd, double yd, double zd, float speed, float spread);
	void lerpTo(double x, double y, double z, float yRot, float xRot, int_t steps) override;
	void lerpMotion(double xd, double yd, double zd) override;
	void tick() override;
	float getShadowHeightOffs() override;
	int_t catchFish();

protected:
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
};
