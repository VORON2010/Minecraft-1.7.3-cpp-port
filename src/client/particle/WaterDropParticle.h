#pragma once

#include "client/particle/Particle.h"

class WaterDropParticle : public Particle
{
public:
	WaterDropParticle(Level &level, double x, double y, double z);

	virtual void tick() override;
};
