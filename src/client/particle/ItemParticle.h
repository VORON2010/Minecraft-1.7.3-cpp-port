#pragma once

#include "client/particle/Particle.h"

class Item;
class Level;
class Tesselator;

class ItemParticle : public Particle
{
public:
	ItemParticle(Level &level, double x, double y, double z, Item &item);
	int_t getParticleTexture() const override;
	void render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2) override;
};
