#include "client/particle/NoteParticle.h"

#include <cmath>

namespace
{
	constexpr float PI_F = 3.14159265358979323846f;
}

NoteParticle::NoteParticle(Level &level, double x, double y, double z, double note)
	: NoteParticle(level, x, y, z, note, 2.0f)
{
}

NoteParticle::NoteParticle(Level &level, double x, double y, double z, double note, float scale)
	: Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
	xd *= 0.01f;
	yd *= 0.01f;
	zd *= 0.01f;
	yd += 0.2;
	rCol = std::sin((static_cast<float>(note) + 0.0f) * PI_F * 2.0f) * 0.65f + 0.35f;
	gCol = std::sin((static_cast<float>(note) + 1.0f / 3.0f) * PI_F * 2.0f) * 0.65f + 0.35f;
	bCol = std::sin((static_cast<float>(note) + 2.0f / 3.0f) * PI_F * 2.0f) * 0.65f + 0.35f;
	size *= 12.0f / 16.0f;
	size *= scale;
	oSize = size;
	lifetime = 6;
	noPhysics = false;
	tex = 64;
}

void NoteParticle::tick()
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
	xd *= 0.66f;
	yd *= 0.66f;
	zd *= 0.66f;
	if (onGround)
	{
		xd *= 0.7f;
		zd *= 0.7f;
	}
}

void NoteParticle::render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2)
{
	float life = (age + a) / lifetime * 32.0f;
	if (life < 0.0f)
		life = 0.0f;
	if (life > 1.0f)
		life = 1.0f;
	size = oSize * life;
	Particle::render(t, a, xa, ya, za, xa2, za2);
}
