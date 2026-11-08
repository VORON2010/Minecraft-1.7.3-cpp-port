#include "world/entity/projectile/ThrownExpBottle.h"

#include <cmath>

#include "world/entity/item/ExperienceOrb.h"
#include "world/level/Level.h"
#include "util/Mth.h"

ThrownExpBottle::ThrownExpBottle(Level &level, Mob &owner) : EntitySnowball(level, owner)
{
	// ThrownEntity with getPower 0.7 and getStartPitchOffset -20
	float speed = 0.4f;
	xd = -Mth::sin(yRot * Mth::DEGRAD) * Mth::cos(xRot * Mth::DEGRAD) * speed;
	zd = Mth::cos(yRot * Mth::DEGRAD) * Mth::cos(xRot * Mth::DEGRAD) * speed;
	yd = -Mth::sin((xRot - 20.0f) * Mth::DEGRAD) * speed;
	shoot(xd, yd, zd, 0.7f, 1.0f);
}

void ThrownExpBottle::onHit(HitResult &hit)
{
	(void)hit;
	if (level.isOnline)
		return;
	level.levelEvent(2002, static_cast<int_t>(std::lround(x)), static_cast<int_t>(std::lround(y)), static_cast<int_t>(std::lround(z)), 0);
	int_t xp = 3 + level.random.nextInt(5) + level.random.nextInt(5);
	while (xp > 0)
	{
		int_t value = ExperienceOrb::getExperienceValue(xp);
		xp -= value;
		level.addEntity(std::make_shared<ExperienceOrb>(level, x, y, z, value));
	}
	remove();
}
