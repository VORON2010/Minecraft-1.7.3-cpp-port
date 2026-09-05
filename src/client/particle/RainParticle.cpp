#include "client/particle/RainParticle.h"

#include "java/Math.h"
#include "util/Mth.h"
#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/tile/LiquidTile.h"


RainParticle::RainParticle(Level &level, double x, double y, double z)
	: Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
	xd *= 0.3f;
	yd = static_cast<float>(Math::random()) * 0.2f + 0.1f;
	zd *= 0.3f;
	rCol = 1.0f;
	gCol = 1.0f;
	bCol = 1.0f;
	tex = 19 + random.nextInt(4);
	setSize(0.01f, 0.01f);
	gravity = 0.06f;
	lifetime = static_cast<int_t>(8.0 / (Math::random() * 0.8 + 0.2));
}


void RainParticle::render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2)
{
	Particle::render(t, a, xa, ya, za, xa2, za2);
}


void RainParticle::tick()
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
		remove();

	if (onGround)
	{
		if (Math::random() < 0.5)
			remove();
		xd *= 0.7f;
		zd *= 0.7f;
	}

	int_t xt = Mth::floor(x);
	int_t yt = Mth::floor(y);
	int_t zt = Mth::floor(z);
	const Material &material = level.getMaterial(xt, yt, zt);
	if (material.isLiquid() || material.isSolid())
	{
		double surface = static_cast<float>(yt + 1) - LiquidTile::getHeight(level.getData(xt, yt, zt));
		if (y < surface)
			remove();
	}
}
