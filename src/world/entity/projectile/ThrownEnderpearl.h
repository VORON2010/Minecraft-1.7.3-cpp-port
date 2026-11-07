#pragma once

#include "world/entity/projectile/EntitySnowball.h"

// 1.0 EnderPearlEntity: teleports the thrower where it lands, 5 points of fall damage
class ThrownEnderpearl : public EntitySnowball
{
public:
	using EntitySnowball::EntitySnowball;
	jstring getEncodeId() const override { return u"ThrownEnderpearl"; }

protected:
	void onHit(HitResult &hit) override;
};
