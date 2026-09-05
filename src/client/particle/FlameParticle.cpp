#include "client/particle/FlameParticle.h"

#include "world/level/Level.h"
#include "util/Mth.h"
#include "java/Random.h"
#include <cmath>
#include <cstdlib>

FlameParticle::FlameParticle(Level &level, double x, double y, double z, double xd, double yd, double zd)
	: Particle(level, x, y, z, xd, yd, zd)
{
	this->xd = this->xd * 0.01f + xd;
	this->yd = this->yd * 0.01f + yd;
	this->zd = this->zd * 0.01f + zd;
	
	
	for (int_t i = 0; i < 3; ++i)
		(void)((random.nextFloat() - random.nextFloat()) * 0.05f);
	oSize = size;
	rCol = gCol = bCol = 1.0f;
	lifetime = (int_t)(8.0 / (random.nextFloat() * 0.8 + 0.2)) + 4;
	noPhysics = true;
	tex = 48;
}

void FlameParticle::render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2)
{
	float s = (age + a) / lifetime;
	size = oSize * (1.0f - s * s * 0.5f);
	Particle::render(t, a, xa, ya, za, xa2, za2);
}

float FlameParticle::getBrightness(float a)
{
	float l = (age + a) / lifetime;
	if (l < 0.0f)
		l = 0.0f;
	if (l > 1.0f)
		l = 1.0f;
	
	float br = Particle::getBrightness(a);
	return br * l + (1.0f - l);
}

void FlameParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;
	if (age++ >= lifetime)
	{
		remove();
		return;
	}
	
	move(xd, yd, zd);
	xd *= 0.96f;
	yd *= 0.96f;
	zd *= 0.96f;
	if (onGround)
	{
		xd *= 0.7f;
		zd *= 0.7f;
	}
}
