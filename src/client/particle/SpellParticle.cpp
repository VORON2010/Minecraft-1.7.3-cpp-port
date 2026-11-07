#include "client/particle/SpellParticle.h"

#include "java/Math.h"

SpellParticle::SpellParticle(Level &level, double x, double y, double z, double xa, double ya, double za)
	: Particle(level, x, y, z, xa, ya, za)
{
	yd *= 0.2;
	if (xa == 0.0 && za == 0.0)
	{
		xd *= 0.1;
		zd *= 0.1;
	}
	size *= 0.75f;
	lifetime = static_cast<int_t>(8.0 / (Math::random() * 0.8 + 0.2));
	noPhysics = false;
	tex = baseTex + 7;
}

void SpellParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;
	if (age++ >= lifetime)
		remove();

	tex = baseTex + (7 - age * 8 / lifetime);
	yd += 0.004;
	double oldY = y;
	move(xd, yd, zd);
	if (y == oldY)
	{
		xd *= 1.1;
		zd *= 1.1;
	}
	xd *= 0.96;
	yd *= 0.96;
	zd *= 0.96;
	if (onGround)
	{
		xd *= 0.7;
		zd *= 0.7;
	}
}
