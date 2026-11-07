#include "client/particle/CritParticle.h"

#include "java/Math.h"

CritParticle::CritParticle(Level &level, double x, double y, double z, double xa, double ya, double za, float scale)
	: Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
	xd = xd * 0.1 + xa * 0.4;
	yd = yd * 0.1 + ya * 0.4;
	zd = zd * 0.1 + za * 0.4;
	rCol = gCol = bCol = static_cast<float>(Math::random() * 0.3 + 0.6);
	size *= 0.75f;
	size *= scale;
	baseSize = size;
	lifetime = static_cast<int_t>(6.0 / (Math::random() * 0.8 + 0.6));
	lifetime = static_cast<int_t>(static_cast<float>(lifetime) * scale);
	noPhysics = false;
	tex = 65;
	tick();
}

void CritParticle::render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2)
{
	float s = (static_cast<float>(age) + a) / static_cast<float>(lifetime) * 32.0f;
	if (s < 0.0f) s = 0.0f;
	if (s > 1.0f) s = 1.0f;
	size = baseSize * s;
	Particle::render(t, a, xa, ya, za, xa2, za2);
}

void CritParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;
	if (age++ >= lifetime)
		remove();

	move(xd, yd, zd);
	gCol *= 0.96f;
	bCol *= 0.9f;
	xd *= 0.7;
	yd *= 0.7;
	zd *= 0.7;
	yd -= 0.02;
	if (onGround)
	{
		xd *= 0.7;
		zd *= 0.7;
	}
}
