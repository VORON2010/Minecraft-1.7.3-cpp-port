#pragma once

#include "client/particle/Particle.h"

// 1.0 potion swirl ("spell" from splash potions, "mobSpell" around mobs with effects)
class SpellParticle : public Particle
{
private:
	int_t baseTex = 128;

public:
	SpellParticle(Level &level, double x, double y, double z, double xa, double ya, double za);

	void setBaseTex(int_t tex) { baseTex = tex; }
	void tick() override;
};
