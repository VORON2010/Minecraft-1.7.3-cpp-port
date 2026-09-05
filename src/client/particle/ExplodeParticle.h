#pragma once

#include "client/particle/Particle.h"

class ExplodeParticle : public Particle
{
public:
	ExplodeParticle(Level &level, double x, double y, double z, double xa, double ya, double za);

	virtual void render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2) override;
	virtual void tick() override;
};
