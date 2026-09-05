#pragma once

#include "client/particle/Particle.h"


class PortalParticle : public Particle
{
private:
	float oSize;  
	double xStart;  
	double yStart;  
	double zStart;  

public:
	PortalParticle(Level &level, double x, double y, double z, double xd, double yd, double zd);

	virtual void tick() override;
	virtual void render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2) override;
	float getBrightness(float a) override;  
};
