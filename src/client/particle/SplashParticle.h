#pragma once

#include "client/particle/WaterDropParticle.h"

class SplashParticle : public WaterDropParticle
{
public:
	SplashParticle(Level &level, double x, double y, double z, double xa, double ya, double za);
};
