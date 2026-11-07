#pragma once

#include "client/particle/Particle.h"

// b1.8 critical hit spark (also used for fully drawn arrows)
class CritParticle : public Particle
{
private:
	float baseSize = 1.0f;

public:
	CritParticle(Level &level, double x, double y, double z, double xa, double ya, double za, float scale = 1.0f);

	void render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2) override;
	void tick() override;
};
