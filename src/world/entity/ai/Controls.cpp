#include "world/entity/ai/Controls.h"

#include <cmath>
#include <algorithm>

#include "world/entity/Mob.h"
#include "world/entity/ai/PathNavigation.h"
#include "util/Mth.h"

float clampedRotate(float from, float to, float max)
{
	float delta = Mth::wrapDegrees(to - from);
	if (delta > max)
		delta = max;
	if (delta < -max)
		delta = -max;
	return from + delta;
}

void LookControl::setLookAt(Entity &entity, float yMax, float xMax)
{
	wantedX = entity.x;
	if (dynamic_cast<Mob *>(&entity) != nullptr)
		wantedY = entity.y + entity.getHeadHeight();
	else
		wantedY = (entity.bb.y0 + entity.bb.y1) / 2.0;
	wantedZ = entity.z;
	this->yMax = yMax;
	this->xMax = xMax;
	hasWanted = true;
}

void LookControl::setLookAt(double x, double y, double z, float yMax, float xMax)
{
	wantedX = x;
	wantedY = y;
	wantedZ = z;
	this->yMax = yMax;
	this->xMax = xMax;
	hasWanted = true;
}

void LookControl::tick()
{
	mob.xRot = 0.0f;
	if (hasWanted)
	{
		hasWanted = false;
		double xd = wantedX - mob.x;
		double yd = wantedY - (mob.y + mob.getHeadHeight());
		double zd = wantedZ - mob.z;
		double sd = Mth::sqrt(xd * xd + zd * zd);
		float yRotD = static_cast<float>(std::atan2(zd, xd) * 180.0 / Mth::PI) - 90.0f;
		float xRotD = static_cast<float>(-(std::atan2(yd, sd) * 180.0 / Mth::PI));
		mob.xRot = clampedRotate(mob.xRot, xRotD, xMax);
		mob.yHeadRot = clampedRotate(mob.yHeadRot, yRotD, yMax);
	}
	else
		mob.yHeadRot = clampedRotate(mob.yHeadRot, mob.yBodyRot, 10.0f);

	float headDiff = Mth::wrapDegrees(mob.yHeadRot - mob.yBodyRot);
	if (!mob.getNavigation().isDone())
	{
		if (headDiff < -75.0f)
			mob.yHeadRot = mob.yBodyRot - 75.0f;
		if (headDiff > 75.0f)
			mob.yHeadRot = mob.yBodyRot + 75.0f;
	}
}

MoveControl::MoveControl(Mob &mob) : mob(mob), wantedX(mob.x), wantedY(mob.y), wantedZ(mob.z)
{
}

void MoveControl::setWantedPosition(double x, double y, double z, float speed)
{
	wantedX = x;
	wantedY = y;
	wantedZ = z;
	this->speed = speed;
	hasWanted = true;
}

void MoveControl::tick()
{
	mob.setYya(0.0f);
	if (!hasWanted)
		return;
	hasWanted = false;
	int_t yFloor = Mth::floor(mob.bb.y0 + 0.5);
	double xd = wantedX - mob.x;
	double zd = wantedZ - mob.z;
	double yd = wantedY - yFloor;
	double dd = xd * xd + yd * yd + zd * zd;
	if (dd < 2.5000003E-7f)
		return;
	float yRotD = static_cast<float>(std::atan2(zd, xd) * 180.0 / Mth::PI) - 90.0f;
	mob.yRot = clampedRotate(mob.yRot, yRotD, 30.0f);
	mob.setSpeed(speed * mob.getSpeedModifier());
	if (yd > 0.0 && xd * xd + zd * zd < 1.0)
		mob.getJumpControl().jumpOnce();
}

void JumpControl::tick()
{
	mob.setJumping(jump);
	jump = false;
}

void BodyControl::clientTick()
{
	double xd = mob.x - mob.xo;
	double zd = mob.z - mob.zo;
	if (xd * xd + zd * zd > 2.5000003E-7f)
	{
		mob.yBodyRot = mob.yRot;
		mob.yHeadRot = [](float a, float b, float max) {
			float d = Mth::wrapDegrees(a - b);
			if (d < -max) d = -max;
			if (d >= max) d = max;
			return a - d;
		}(mob.yBodyRot, mob.yHeadRot, 75.0f);
		lastHeadY = mob.yHeadRot;
		timeStill = 0;
		return;
	}

	float maxDiff = 75.0f;
	if (std::abs(mob.yHeadRot - lastHeadY) > 15.0f)
	{
		timeStill = 0;
		lastHeadY = mob.yHeadRot;
	}
	else
	{
		timeStill++;
		if (timeStill > 10)
			maxDiff = std::max(1.0f - (timeStill - 10) / 10.0f, 0.0f) * 75.0f;
	}

	float d = Mth::wrapDegrees(mob.yHeadRot - mob.yBodyRot);
	if (d < -maxDiff)
		d = -maxDiff;
	if (d >= maxDiff)
		d = maxDiff;
	mob.yBodyRot = mob.yHeadRot - d;
}
