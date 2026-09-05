#include "client/particle/BubbleParticle.h"

#include "util/Mth.h"
#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/material/LiquidMaterial.h"

BubbleParticle::BubbleParticle(Level &level, double x, double y, double z, double xa, double ya, double za)
	: Particle(level, x, y, z, xa, ya, za)
{
	rCol = 1.0f;
	gCol = 1.0f;
	bCol = 1.0f;
	tex = 32;
	setSize(0.02f, 0.02f);
	size = size * (random.nextFloat() * 0.6f + 0.2f);
	xd = xa * 0.2f + (float)((random.nextFloat() * 2.0 - 1.0) * 0.02f);
	yd = ya * 0.2f + (float)((random.nextFloat() * 2.0 - 1.0) * 0.02f);
	zd = za * 0.2f + (float)((random.nextFloat() * 2.0 - 1.0) * 0.02f);
	lifetime = (int_t)(8.0 / (random.nextFloat() * 0.8 + 0.2));
}

void BubbleParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;
	yd += 0.002;
	move(xd, yd, zd);
	xd *= 0.85f;
	yd *= 0.85f;
	zd *= 0.85f;

	const Material &mat = level.getMaterial(Mth::floor(x), Mth::floor(y), Mth::floor(z));
	const Material &waterMat = Material::water;
	if (&mat != &waterMat)
	{
		remove();
		return;
	}

	if (lifetime-- <= 0)
	{
		remove();
	}
}
