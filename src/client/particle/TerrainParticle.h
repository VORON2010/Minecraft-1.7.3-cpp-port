#pragma once

#include "client/particle/Particle.h"

#include "java/Type.h"

class Level;
class Tile;
class Tesselator;

class TerrainParticle : public Particle
{
private:
	Tile *tile;

public:
	TerrainParticle(Level &level, double x, double y, double z, double xa, double ya, double za, Tile *tile, int_t face = 0, int_t data = 0);

	TerrainParticle &init(int_t x, int_t y, int_t z);

	virtual int_t getParticleTexture() const override;
	virtual void render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2) override;
};
