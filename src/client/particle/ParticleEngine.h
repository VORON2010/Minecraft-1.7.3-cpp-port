#pragma once

#include <vector>
#include <memory>

#include "client/particle/Particle.h"

#include "java/Type.h"
#include "java/String.h"
#include "java/Random.h"

class Level;
class Textures;
class Entity;
class Tile;
class Tesselator;

class ParticleEngine
{
public:
	static constexpr int_t MISC_TEXTURE = 0;
	static constexpr int_t TERRAIN_TEXTURE = 1;
	static constexpr int_t ITEM_TEXTURE = 2;
	static constexpr int_t ENTITY_PARTICLE_TEXTURE = 3;
	static constexpr int_t TEXTURE_COUNT = 4;

private:
	Level *level;
	std::vector<std::unique_ptr<Particle>> particles[4];
	Textures *textures;
	Random random;

public:
	ParticleEngine(Level *level, Textures *textures);

	void add(std::unique_ptr<Particle> p);
	void tick();
	void render(Entity &player, float a);
	void renderLit(Entity &player, float a);
	void setLevel(Level *level);
	void destroy(int_t x, int_t y, int_t z);
	void destroy(int_t x, int_t y, int_t z, int_t tileId, int_t data);
	void crack(int_t x, int_t y, int_t z, int_t face);
	jstring countParticles();
};
