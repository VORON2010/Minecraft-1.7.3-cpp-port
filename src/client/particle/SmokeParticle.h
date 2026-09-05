#pragma once

#include "client/particle/Particle.h"


class SmokeParticle : public Particle
{
private:
	float oSize;  

public:
	SmokeParticle(Level &level, double x, double y, double z, double xa, double ya, double za);
	SmokeParticle(Level &level, double x, double y, double z, double xa, double ya, double za, float scale);
	
	virtual void tick() override;
	virtual void render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2) override;
};
