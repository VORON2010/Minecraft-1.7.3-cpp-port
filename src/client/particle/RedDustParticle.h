#pragma once

#include "client/particle/Particle.h"


class RedDustParticle : public Particle
{
private:
	float oSize;  

public:
	RedDustParticle(Level &level, double x, double y, double z);
	RedDustParticle(Level &level, double x, double y, double z, float scale);
	
	virtual void tick() override;
	virtual void render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2) override;
};
