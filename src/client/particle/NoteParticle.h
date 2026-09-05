#pragma once

#include "client/particle/Particle.h"

class NoteParticle : public Particle
{
private:
	float oSize = 1.0f;

public:
	NoteParticle(Level &level, double x, double y, double z, double note);
	NoteParticle(Level &level, double x, double y, double z, double note, float scale);

	void tick() override;
	void render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2) override;
};
