#pragma once

#include "world/entity/Entity.h"

#include "client/renderer/Tesselator.h"

#include "java/Type.h"

class Level;
class Tesselator;

class Particle : public Entity
{
public:
	static double xOff;
	static double yOff;
	static double zOff;

protected:
	int_t tex = 0;
	float uo = 0.0f;
	float vo = 0.0f;
	int_t age = 0;
	int_t lifetime = 0;
	float size = 1.0f;
	float gravity = 1.0f;
	float rCol = 1.0f;
	float gCol = 1.0f;
	float bCol = 1.0f;

public:
	Particle(Level &level, double x, double y, double z, double xa, double ya, double za);

	virtual ~Particle() {}

	Particle &setPower(float power);
	Particle &scale(float scale);

	virtual void tick() override;

	virtual void render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2);

	virtual int_t getParticleTexture() const;
};
