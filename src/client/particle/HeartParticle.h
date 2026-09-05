#pragma once

#include "client/particle/Particle.h"

class HeartParticle : public Particle
{
private:
	float oSize = 1.0f;

public:
	HeartParticle(Level &level, double x, double y, double z, double xa, double ya, double za);
	HeartParticle(Level &level, double x, double y, double z, double xa, double ya, double za, float scale);

	void tick() override;
	void render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2) override;
};
