#pragma once

#include "client/particle/Particle.h"

#include <memory>

#include "java/Type.h"

class Level;
class Entity;
class Tesselator;

class TakeAnimationParticle : public Particle
{
private:
	std::shared_ptr<Entity> item;
	std::shared_ptr<Entity> target;
	int_t life = 0;
	int_t lifeTime = 0;
	float yOffs;

public:
	TakeAnimationParticle(Level &level, std::shared_ptr<Entity> item, std::shared_ptr<Entity> target, float yOffs);

	void tick() override;
	void render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2) override;
	int_t getParticleTexture() const override;
};
