#include "client/particle/WaterDropParticle.h"

#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/tile/LiquidTile.h"
#include "util/Mth.h"

WaterDropParticle::WaterDropParticle(Level &level, double x, double y, double z)
	: Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
	xd *= 0.3f;
	yd = random.nextFloat() * 0.2f + 0.1f;
	zd *= 0.3f;
	rCol = 1.0f;
	gCol = 1.0f;
	bCol = 1.0f;
	tex = 19 + random.nextInt(4);
	setSize(0.01f, 0.01f);
	gravity = 0.06f;
	lifetime = (int_t)(8.0 / (random.nextFloat() * 0.8 + 0.2));
}

void WaterDropParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;
	yd -= gravity;
	move(xd, yd, zd);
	xd *= 0.98f;
	yd *= 0.98f;
	zd *= 0.98f;

	if (lifetime-- <= 0)
	{
		remove();
		return;
	}

	if (onGround)
	{
		if (random.nextFloat() < 0.5f)
		{
			remove();
			return;
		}
		xd *= 0.7f;
		zd *= 0.7f;
	}

	const Material &m = level.getMaterial(Mth::floor(x), Mth::floor(y), Mth::floor(z));
	if (m.isLiquid() || m.isSolid())
	{
		double surfaceY = Mth::floor(y) + 1 - LiquidTile::getHeight(level.getData(Mth::floor(x), Mth::floor(y), Mth::floor(z)));
		if (y < surfaceY)
			remove();
	}
}
