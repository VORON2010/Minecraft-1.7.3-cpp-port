#include "client/particle/HeartParticle.h"

HeartParticle::HeartParticle(Level &level, double x, double y, double z, double xa, double ya, double za)
	: HeartParticle(level, x, y, z, xa, ya, za, 2.0f)
{
}

HeartParticle::HeartParticle(Level &level, double x, double y, double z, double xa, double ya, double za, float scale)
	: Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
	xd *= 0.01f;
	yd *= 0.01f;
	zd *= 0.01f;
	yd += 0.1;
	size *= 12.0f / 16.0f;
	size *= scale;
	oSize = size;
	lifetime = 16;
	noPhysics = false;
	tex = 80;
}

void HeartParticle::tick()
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
	if (y == yo)
	{
		xd *= 1.1;
		zd *= 1.1;
	}
	xd *= 0.86f;
	yd *= 0.86f;
	zd *= 0.86f;
	if (onGround)
	{
		xd *= 0.7f;
		zd *= 0.7f;
	}
}

void HeartParticle::render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2)
{
	float life = (age + a) / lifetime * 32.0f;
	if (life < 0.0f)
		life = 0.0f;
	if (life > 1.0f)
		life = 1.0f;
	size = oSize * life;
	Particle::render(t, a, xa, ya, za, xa2, za2);
}
