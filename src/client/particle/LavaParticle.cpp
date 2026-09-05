#include "client/particle/LavaParticle.h"

#include "world/level/Level.h"
#include "util/Mth.h"
#include "java/Random.h"
#include <cmath>
#include <cstdlib>

LavaParticle::LavaParticle(Level &level, double x, double y, double z)
	: Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
	xd *= 0.8f;
	yd *= 0.8f;
	zd *= 0.8f;
	yd = random.nextFloat() * 0.4f + 0.05f;
	rCol = gCol = bCol = 1.0f;
	size = size * (random.nextFloat() * 2.0f + 0.2f);
	oSize = size;
	lifetime = (int_t)(16.0 / (random.nextFloat() * 0.8 + 0.2));
	noPhysics = false;
	tex = 49;
}

float LavaParticle::getBrightness(float a)
{
	return 1.0f;
}

void LavaParticle::render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2)
{
	float s = (age + a) / lifetime;
	size = oSize * (1.0f - s * s);
	Particle::render(t, a, xa, ya, za, xa2, za2);
}

void LavaParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;
	if (age++ >= lifetime)
	{
		remove();
		return;
	}
	
	float odds = (float)age / lifetime;
	if (random.nextFloat() > odds)
	{
		level.addParticle(u"smoke", x, y, z, xd, yd, zd);
	}
	
	yd -= 0.03;
	move(xd, yd, zd);
	xd *= 0.999f;
	yd *= 0.999f;
	zd *= 0.999f;
	if (onGround)
	{
		xd *= 0.7f;
		zd *= 0.7f;
	}
}
