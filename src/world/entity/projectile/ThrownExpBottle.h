#pragma once

#include "world/entity/projectile/EntitySnowball.h"

// 1.5.2 ExperienceBottleEntity: breaks into 3-11 xp
class ThrownExpBottle : public EntitySnowball
{
public:
	explicit ThrownExpBottle(Level &level) : EntitySnowball(level) {}
	ThrownExpBottle(Level &level, Mob &owner);

	jstring getEncodeId() const override { return u"ThrownExpBottle"; }

protected:
	float getGravity() const override { return 0.07f; }
	void onHit(HitResult &hit) override;
};
