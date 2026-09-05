#pragma once

#include "client/particle/Particle.h"

class RainParticle : public Particle
{
public:
	RainParticle(Level &level, double x, double y, double z);

	void render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2) override;
	void tick() override;
};
