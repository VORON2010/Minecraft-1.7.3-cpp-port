#pragma once

#include "client/particle/Particle.h"

class BubbleParticle : public Particle
{
public:
	BubbleParticle(Level &level, double x, double y, double z, double xa, double ya, double za);

	virtual void tick() override;
};
