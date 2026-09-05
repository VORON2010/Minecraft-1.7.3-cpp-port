#pragma once

#include "world/entity/Entity.h"

class EntityBoat : public Entity
{
public:
	int_t boatCurrentDamage = 0;
	int_t boatTimeSinceHit = 0;
	int_t boatRockDirection = 1;

private:
	int_t lerpSteps = 0;
	double lerpX = 0.0;
	double lerpY = 0.0;
	double lerpZ = 0.0;
	float lerpYaw = 0.0f;
	float lerpPitch = 0.0f;
	double lerpXd = 0.0;
	double lerpYd = 0.0;
	double lerpZd = 0.0;

public:
	EntityBoat(Level &level);
	EntityBoat(Level &level, double x, double y, double z);

	jstring getEncodeId() const override { return u"Boat"; }
	bool isPickable() override { return !removed; }
	bool isPushable() override { return true; }
	AABB *getCollideAgainstBox(Entity &entity) override;
	AABB *getCollideBox() override;
	double getRideHeight() override;
	bool hurt(Entity *source, int_t dmg) override;
	bool interact(Player &player) override;
	void tick() override;
	void animateHurt() override;
	void positionRider() override;
	void lerpTo(double x, double y, double z, float yRot, float xRot, int_t steps) override;
	void lerpMotion(double x, double y, double z) override;
	float getShadowHeightOffs() override;

protected:
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
};
