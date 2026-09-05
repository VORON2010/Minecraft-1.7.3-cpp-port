#pragma once

#include "client/particle/Particle.h"

class FlameParticle : public Particle
{
private:
	float oSize;

public:
	FlameParticle(Level &level, double x, double y, double z, double xd, double yd, double zd);

	virtual void render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2) override;
	float getBrightness(float a) override;  
	virtual void tick() override;
};
