#include "client/particle/SplashParticle.h"

#include "world/level/Level.h"

SplashParticle::SplashParticle(Level &level, double x, double y, double z, double xa, double ya, double za)
	: WaterDropParticle(level, x, y, z)
{
	gravity = 0.04f;
	tex++;
	if (ya == 0.0 && (xa != 0.0 || za != 0.0))
	{
		xd = xa;
		yd = ya + 0.1;
		zd = za;
	}
}
