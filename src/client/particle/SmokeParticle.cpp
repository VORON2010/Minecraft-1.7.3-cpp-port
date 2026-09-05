#include "client/particle/SmokeParticle.h"

#include "world/level/Level.h"
#include "util/Mth.h"
#include "java/Random.h"
#include <cmath>


SmokeParticle::SmokeParticle(Level &level, double x, double y, double z, double xa, double ya, double za)
	: SmokeParticle(level, x, y, z, xa, ya, za, 1.0f)
{
	
}

SmokeParticle::SmokeParticle(Level &level, double x, double y, double z, double xa, double ya, double za, float scale)
	: Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
	
	xd *= 0.1f;  
	yd *= 0.1f;  
	zd *= 0.1f;  
	xd += xa;  
	yd += ya;  
	zd += za;  
	
	
	float gray = (float)(random.nextFloat() * 0.3f);
	rCol = gray;
	gCol = gray;
	bCol = gray;
	
	size *= 0.75f;  
	size *= scale;  
	oSize = size;  
	
	
	lifetime = (int_t)(8.0 / (random.nextFloat() * 0.8 + 0.2));
	lifetime = (int_t)(lifetime * scale);
	
	noPhysics = false;  
}

void SmokeParticle::tick()
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
	
	if (y == yo)  
	{
		xd *= 1.1;  
		zd *= 1.1;  
	}
	
	xd *= 0.96f;  
	yd *= 0.96f;  
	zd *= 0.96f;  
	
	if (onGround)  
	{
		xd *= 0.7f;  
		zd *= 0.7f;  
	}
}

void SmokeParticle::render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2)
{
	
	float l = (age + a) / lifetime * 32.0f;  
	if (l < 0.0f)  
		l = 0.0f;  
	if (l > 1.0f)  
		l = 1.0f;  
	
	size = oSize * l;  
	Particle::render(t, a, xa, ya, za, xa2, za2);  
}
