#include "client/particle/RedDustParticle.h"

#include "world/level/Level.h"
#include "util/Mth.h"
#include "java/Random.h"
#include <cmath>


RedDustParticle::RedDustParticle(Level &level, double x, double y, double z)
	: RedDustParticle(level, x, y, z, 1.0f)
{
	
}

RedDustParticle::RedDustParticle(Level &level, double x, double y, double z, float scale)
	: Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
	
	xd *= 0.1f;  
	yd *= 0.1f;  
	zd *= 0.1f;  
	
	
	rCol = (float)(random.nextFloat() * 0.3f) + 0.7f;  
	gCol = bCol = (float)(random.nextFloat() * 0.1f);  
	
	size *= 0.75f;  
	size *= scale;  
	oSize = size;  
	
	
	lifetime = (int_t)(8.0 / (random.nextFloat() * 0.8 + 0.2));
	lifetime = (int_t)(lifetime * scale);
	
	noPhysics = false;  
}

void RedDustParticle::tick()
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

void RedDustParticle::render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2)
{
	
	float l = (age + a) / lifetime * 32.0f;  
	if (l < 0.0f)  
		l = 0.0f;  
	if (l > 1.0f)  
		l = 1.0f;  
	
	size = oSize * l;  
	Particle::render(t, a, xa, ya, za, xa2, za2);  
}
