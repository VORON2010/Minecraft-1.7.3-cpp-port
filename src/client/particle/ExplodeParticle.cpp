#include "client/particle/ExplodeParticle.h"

#include "world/level/Level.h"
#include "util/Mth.h"
#include "java/Random.h"
#include <cmath>
#include <cstdlib>

ExplodeParticle::ExplodeParticle(Level &level, double x, double y, double z, double xa, double ya, double za)
	: Particle(level, x, y, z, xa, ya, za)
{
	xd = xa + (float)(random.nextFloat() * 2.0 - 1.0) * 0.05f;
	yd = ya + (float)(random.nextFloat() * 2.0 - 1.0) * 0.05f;
	zd = za + (float)(random.nextFloat() * 2.0 - 1.0) * 0.05f;
	float col = random.nextFloat() * 0.3f + 0.7f;
	rCol = col;
	gCol = col;
	bCol = col;
	size = random.nextFloat() * random.nextFloat() * 6.0f + 1.0f;
	lifetime = (int_t)(16.0 / (random.nextFloat() * 0.8 + 0.2)) + 2;
}

void ExplodeParticle::render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2)
{
	Particle::render(t, a, xa, ya, za, xa2, za2);
}

void ExplodeParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;
	if (age++ >= lifetime)
	{
		remove();
		return;
	}
	
	tex = 7 - age * 8 / lifetime;
	
	yd += 0.004;
	move(xd, yd, zd);
	
	xd *= 0.9f;
	yd *= 0.9f;
	zd *= 0.9f;
	
	if (onGround)
	{
		xd *= 0.7f;
		zd *= 0.7f;
	}
}
