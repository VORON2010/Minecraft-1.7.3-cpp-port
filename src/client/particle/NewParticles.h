#pragma once

#include "client/particle/Particle.h"

class Textures;
class Material;

// Particles added in b1.8 and 1.0, ported from the 1.0 client.

class SuspendedParticle : public Particle
{
public:
	SuspendedParticle(Level &level, double x, double y, double z, double xa, double ya, double za);
	void tick() override;
};

// "depthsuspend" (void fog) and "townaura" (mycelium)
class DepthSuspendParticle : public Particle
{
public:
	DepthSuspendParticle(Level &level, double x, double y, double z, double xa, double ya, double za);
	void tick() override;
};

// flying runes from bookshelves to the enchanting table
class EnchantingParticle : public Particle
{
private:
	double startX, startY, startZ;

public:
	EnchantingParticle(Level &level, double x, double y, double z, double xa, double ya, double za);
	float getBrightness(float a) override;
	void tick() override;
};

class CloudParticle : public Particle
{
private:
	float baseSize;

public:
	CloudParticle(Level &level, double x, double y, double z, double xa, double ya, double za);
	void render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2) override;
	void tick() override;
};

// water / lava dripping through the ceiling
class DripParticle : public Particle
{
private:
	bool lava;
	int_t stuckTime = 40;

public:
	DripParticle(Level &level, double x, double y, double z, bool lava);
	float getBrightness(float a) override;
	void tick() override;
};

// spawns large explosion puffs for a few ticks; draws nothing itself
class HugeExplosionParticle : public Particle
{
private:
	int_t life = 0;
	int_t maxLife = 8;

public:
	HugeExplosionParticle(Level &level, double x, double y, double z);
	void render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2) override;
	void tick() override;
};

// animated sprite from /misc/explosion.png
class LargeExplosionParticle : public Particle
{
private:
	Textures &textures;
	int_t life = 0;
	int_t maxLife = 0;
	float scaleFactor = 1.0f;

public:
	LargeExplosionParticle(Textures &textures, Level &level, double x, double y, double z, double progress);
	void render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2) override;
	void tick() override;
	int_t getParticleTexture() const override { return 3; }
};
