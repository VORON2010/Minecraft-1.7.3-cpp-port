#include "world/entity/PrimedTNT.h"

#include <cmath>
#include "world/level/Level.h"
#include "world/level/Explosion.h"
#include "nbt/CompoundTag.h"
#include "util/Mth.h"

PrimedTNT::PrimedTNT(Level &level) : Entity(level)
{
	setSize(0.98f, 0.98f);
	heightOffset = bbHeight / 2.0f;
	makeStepSound = false;
}

PrimedTNT::PrimedTNT(Level &level, double x, double y, double z) : PrimedTNT(level)
{
	setPos(x, y, z);
	float angle = static_cast<float>(level.random.nextFloat() * Mth::PI * 2.0);
	xd = -Mth::sin(angle) * 0.02f;
	yd = 0.2f;
	zd = -Mth::cos(angle) * 0.02f;
	fuse = 80;
	xOld = x;
	yOld = y;
	zOld = z;
}

void PrimedTNT::tick()
{
	xOld = x;
	yOld = y;
	zOld = z;
	yd -= 0.04f;
	move(xd, yd, zd);
	xd *= 0.98f;
	yd *= 0.98f;
	zd *= 0.98f;
	if (onGround)
	{
		xd *= 0.7f;
		zd *= 0.7f;
		yd *= -0.5f;
	}

	if (--fuse <= 0)
	{
		if (!level.isOnline)
		{
			remove();
			level.createExplosion(this, x, y, z, 4.0f);
		}
		else
		{
			remove();
		}
	}
	else
	{
		level.addParticle(u"smoke", x, y + 0.5, z, 0.0, 0.0, 0.0);
	}
}

void PrimedTNT::addAdditionalSaveData(CompoundTag &tag)
{
	tag.putByte(u"Fuse", static_cast<byte_t>(fuse));
}

void PrimedTNT::readAdditionalSaveData(CompoundTag &tag)
{
	fuse = tag.getByte(u"Fuse");
}
