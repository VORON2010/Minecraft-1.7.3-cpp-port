#include "client/particle/PortalParticle.h"

#include "world/level/Level.h"
#include "util/Mth.h"
#include "java/Random.h"
#include <cmath>
#include <cstdlib>


PortalParticle::PortalParticle(Level &level, double x, double y, double z, double xd, double yd, double zd)
	: Particle(level, x, y, z, xd, yd, zd)
{
	
	this->xd = xd;  
	this->yd = yd;  
	this->zd = zd;  

	xStart = this->x = x;  
	yStart = this->y = y;  
	zStart = this->z = z;  

	
	float br = random.nextFloat() * 0.6f + 0.4f;  
	oSize = size = random.nextFloat() * 0.2f + 0.5f;  

	
	rCol = gCol = bCol = 1.0f * br;  
	gCol *= 0.3f;  
	rCol *= 0.9f;  

	lifetime = (int_t)(random.nextFloat() * 10.0) + 40;  
	noPhysics = true;  
	tex = (int_t)(random.nextFloat() * 8.0);  
}

void PortalParticle::render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2)
{
	
	float s = (age + a) / lifetime;  
	s = 1.0f - s;  
	s *= s;  
	s = 1.0f - s;  
	size = oSize * s;  
	Particle::render(t, a, xa, ya, za, xa2, za2);  
}

float PortalParticle::getBrightness(float a)
{
	
	float br = Entity::getBrightness(a);  
	float pos = (float)age / lifetime;  
	pos *= pos;  
	pos *= pos;  
	return br * (1.0f - pos) + pos;  
}

void PortalParticle::tick()
{
	
	xo = x;  
	yo = y;  
	zo = z;  

	float pos = (float)age / lifetime;  
	float var3 = -pos + pos * pos * 2.0f;  
	float var4 = 1.0f - var3;  

	
	x = xStart + xd * var4;
	y = yStart + yd * var4 + (1.0f - pos);
	z = zStart + zd * var4;

	if (age++ >= lifetime)  
	{
		remove();  
		return;
	}
}
