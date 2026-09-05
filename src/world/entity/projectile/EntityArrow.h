#pragma once

#include <memory>

#include "world/entity/Entity.h"

class Mob;
class Player;

class EntityArrow : public Entity
{
protected:
	int_t xTile = -1;
	int_t yTile = -1;
	int_t zTile = -1;
	int_t inTile = 0;
	int_t inData = 0;
	bool inGround = false;
	int_t ticksInGround = 0;
	int_t ticksInAir = 0;

public:
	bool doesArrowBelongToPlayer = false;
	int_t arrowShake = 0;
	std::weak_ptr<Entity> owner;

	EntityArrow(Level &level);
	EntityArrow(Level &level, double x, double y, double z);
	EntityArrow(Level &level, Mob &owner);

	jstring getEncodeId() const override { return u"Arrow"; }
	void setArrowHeading(double xd, double yd, double zd, float speed, float spread);
	void setVelocity(double xd, double yd, double zd);
	void lerpMotion(double xd, double yd, double zd) override { setVelocity(xd, yd, zd); }
	void tick() override;
	void playerTouch(Player &player) override;

protected:
	void addAdditionalSaveData(CompoundTag &tag) override;
	void readAdditionalSaveData(CompoundTag &tag) override;
};
