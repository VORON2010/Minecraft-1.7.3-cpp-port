#pragma once

#include "world/entity/projectile/EntityFireball.h"

// 1.0 blaze fireball: sets things on fire instead of exploding
class SmallFireball : public EntityFireball
{
public:
	SmallFireball(Level &level);
	SmallFireball(Level &level, Mob &owner, double xPower, double yPower, double zPower);
	jstring getEncodeId() const override { return u"SmallFireball"; }
	bool isPickable() override;
	bool hurt(Entity *source, int_t dmg) override;

protected:
	void onHit(HitResult &hit) override;
};
