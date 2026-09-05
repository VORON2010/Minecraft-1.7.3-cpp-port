#pragma once

#include "client/particle/Particle.h"

class LavaParticle : public Particle
{
private:
	float oSize;

public:
	LavaParticle(Level &level, double x, double y, double z);

	float getBrightness(float a) override;  
	virtual void render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2) override;
	virtual void tick() override;
};
