#pragma once

#include "world/entity/animal/Animal.h"

class Squid : public Animal
{
public:
	float xBodyRot = 0.0f;
	float xBodyRotO = 0.0f;
	float zBodyRot = 0.0f;
	float zBodyRotO = 0.0f;
	float tentacleMovement = 0.0f;
	float oldTentacleMovement = 0.0f;
	float tentacleAngle = 0.0f;
	float oldTentacleAngle = 0.0f;

private:
	float speed = 0.0f;
	float tentacleSpeed = 0.0f;
	float rotateSpeed = 0.0f;
	float tx = 0.0f;
	float ty = 0.0f;
	float tz = 0.0f;

public:
	Squid(Level &level);
	jstring getEncodeId() const override { return u"Squid"; }
	bool isInWater() override;
	bool isWaterMob() override;
	void aiStep() override;
	void travel(float var1, float var2) override;
	bool canSpawn() override;

protected:
	jstring getAmbientSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	float getSoundVolume() override;
	int_t getDeathLoot() override;
	void dropDeathLoot() override;
	void updateAi() override;
};
