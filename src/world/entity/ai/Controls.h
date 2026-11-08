#pragma once

#include "java/Type.h"

class Mob;
class Entity;

// 1.5.2 LookControl: turns the head (yHeadRot, xRot) towards a point
class LookControl
{
	Mob &mob;
	float yMax = 0.0f;
	float xMax = 0.0f;
	bool hasWanted = false;
	double wantedX = 0.0, wantedY = 0.0, wantedZ = 0.0;

public:
	explicit LookControl(Mob &mob) : mob(mob) {}
	void setLookAt(Entity &entity, float yMax, float xMax);
	void setLookAt(double x, double y, double z, float yMax, float xMax);
	void tick();
};

// 1.5.2 MoveControl: walks straight at the wanted point and jumps a step up
class MoveControl
{
	Mob &mob;
	double wantedX, wantedY, wantedZ;
	float speed = 0.0f;
	bool hasWanted = false;

public:
	explicit MoveControl(Mob &mob);
	bool hasWantedPosition() const { return hasWanted; }
	float getSpeed() const { return speed; }
	void setWantedPosition(double x, double y, double z, float speed);
	void tick();
};

// 1.5.2 JumpControl
class JumpControl
{
	Mob &mob;
	bool jump = false;

public:
	explicit JumpControl(Mob &mob) : mob(mob) {}
	void jumpOnce() { jump = true; }
	void tick();
};

// 1.5.2 BodyControl: the body follows the head when standing still
class BodyControl
{
	Mob &mob;
	int_t timeStill = 0;
	float lastHeadY = 0.0f;

public:
	explicit BodyControl(Mob &mob) : mob(mob) {}
	void clientTick();
};

float clampedRotate(float from, float to, float max);
