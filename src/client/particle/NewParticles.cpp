#include "client/particle/NewParticles.h"

#include "OpenGL.h"
#include "client/Lighting.h"
#include "client/renderer/Textures.h"
#include "java/Math.h"
#include "util/Mth.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/level/tile/LiquidTile.h"

SuspendedParticle::SuspendedParticle(Level &level, double x, double y, double z, double xa, double ya, double za)
	: Particle(level, x, y - 0.125, z, xa, ya, za)
{
	rCol = 0.4f;
	gCol = 0.4f;
	bCol = 0.7f;
	tex = 0;
	setSize(0.01f, 0.01f);
	size *= random.nextFloat() * 0.6f + 0.2f;
	xd = xa * 0.0;
	yd = ya * 0.0;
	zd = za * 0.0;
	lifetime = static_cast<int_t>(16.0 / (Math::random() * 0.8 + 0.2));
}

void SuspendedParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;
	move(xd, yd, zd);
	if (&level.getMaterial(Mth::floor(x), Mth::floor(y), Mth::floor(z)) != static_cast<const Material *>(&Material::water))
		remove();
	if (lifetime-- <= 0)
		remove();
}

DepthSuspendParticle::DepthSuspendParticle(Level &level, double x, double y, double z, double xa, double ya, double za)
	: Particle(level, x, y, z, xa, ya, za)
{
	float c = random.nextFloat() * 0.1f + 0.2f;
	rCol = gCol = bCol = c;
	tex = 0;
	setSize(0.02f, 0.02f);
	size *= random.nextFloat() * 0.6f + 0.5f;
	xd *= 0.02;
	yd *= 0.02;
	zd *= 0.02;
	lifetime = static_cast<int_t>(20.0 / (Math::random() * 0.8 + 0.2));
	noPhysics = true;
}

void DepthSuspendParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;
	move(xd, yd, zd);
	xd *= 0.99;
	yd *= 0.99;
	zd *= 0.99;
	if (lifetime-- <= 0)
		remove();
}

EnchantingParticle::EnchantingParticle(Level &level, double x, double y, double z, double xa, double ya, double za)
	: Particle(level, x, y, z, xa, ya, za)
{
	xd = xa;
	yd = ya;
	zd = za;
	startX = this->x = x;
	startY = this->y = y;
	startZ = this->z = z;
	float br = random.nextFloat() * 0.6f + 0.4f;
	size = random.nextFloat() * 0.5f + 0.2f;
	rCol = gCol = bCol = br;
	gCol *= 0.9f;
	rCol *= 0.9f;
	lifetime = static_cast<int_t>(Math::random() * 10.0) + 30;
	noPhysics = true;
	tex = static_cast<int_t>(Math::random() * 26.0 + 1.0 + 224.0);
}

float EnchantingParticle::getBrightness(float a)
{
	float br = Particle::getBrightness(a);
	float t = static_cast<float>(age) / static_cast<float>(lifetime);
	t *= t;
	t *= t;
	return br * (1.0f - t) + t;
}

void EnchantingParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;
	float t = 1.0f - static_cast<float>(age) / static_cast<float>(lifetime);
	float fall = 1.0f - t;
	fall *= fall;
	fall *= fall;
	x = startX + xd * t;
	y = startY + yd * t - fall * 1.2f;
	z = startZ + zd * t;
	if (age++ >= lifetime)
		remove();
}

CloudParticle::CloudParticle(Level &level, double x, double y, double z, double xa, double ya, double za)
	: Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
	float scale = 2.5f;
	xd = xd * 0.1 + xa;
	yd = yd * 0.1 + ya;
	zd = zd * 0.1 + za;
	rCol = gCol = bCol = 1.0f - static_cast<float>(Math::random() * 0.3);
	size *= 0.75f;
	size *= scale;
	baseSize = size;
	lifetime = static_cast<int_t>(8.0 / (Math::random() * 0.8 + 0.3));
	lifetime = static_cast<int_t>(static_cast<float>(lifetime) * scale);
	noPhysics = false;
}

void CloudParticle::render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2)
{
	float s = (static_cast<float>(age) + a) / static_cast<float>(lifetime) * 32.0f;
	if (s < 0.0f) s = 0.0f;
	if (s > 1.0f) s = 1.0f;
	size = baseSize * s;
	Particle::render(t, a, xa, ya, za, xa2, za2);
}

void CloudParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;
	if (age++ >= lifetime)
		remove();
	tex = 7 - age * 8 / lifetime;
	move(xd, yd, zd);
	xd *= 0.96;
	yd *= 0.96;
	zd *= 0.96;
	std::shared_ptr<Player> player = level.getNearestPlayer(*this, 2.0);
	if (player != nullptr && y > player->bb.y0)
	{
		y += (player->bb.y0 - y) * 0.2;
		yd += (player->yd - yd) * 0.2;
		setPos(x, y, z);
	}
	if (onGround)
	{
		xd *= 0.7;
		zd *= 0.7;
	}
}

DripParticle::DripParticle(Level &level, double x, double y, double z, bool lava)
	: Particle(level, x, y, z, 0.0, 0.0, 0.0), lava(lava)
{
	xd = yd = zd = 0.0;
	if (!lava)
	{
		rCol = 0.0f;
		gCol = 0.0f;
		bCol = 1.0f;
	}
	else
	{
		rCol = 1.0f;
		gCol = 0.0f;
		bCol = 0.0f;
	}
	tex = 113;
	setSize(0.01f, 0.01f);
	gravity = 0.06f;
	lifetime = static_cast<int_t>(64.0 / (Math::random() * 0.8 + 0.2));
}

float DripParticle::getBrightness(float a)
{
	return lava ? 1.0f : Particle::getBrightness(a);
}

void DripParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;
	if (!lava)
	{
		rCol = 0.2f;
		gCol = 0.3f;
		bCol = 1.0f;
	}
	else
	{
		rCol = 1.0f;
		gCol = 16.0f / static_cast<float>(40 - stuckTime + 16);
		bCol = 4.0f / static_cast<float>(40 - stuckTime + 8);
	}
	yd -= gravity;
	if (stuckTime-- > 0)
	{
		xd *= 0.02;
		yd *= 0.02;
		zd *= 0.02;
		tex = 113;
	}
	else
	{
		tex = 112;
	}
	move(xd, yd, zd);
	xd *= 0.98;
	yd *= 0.98;
	zd *= 0.98;
	if (lifetime-- <= 0)
		remove();
	if (onGround)
	{
		if (!lava)
		{
			remove();
			level.addParticle(u"splash", x, y, z, 0.0, 0.0, 0.0);
		}
		else
		{
			tex = 114;
		}
		xd *= 0.7;
		zd *= 0.7;
	}
	int_t bx = Mth::floor(x), by = Mth::floor(y), bz = Mth::floor(z);
	const Material &m = level.getMaterial(bx, by, bz);
	if (m.isLiquid() || m.isSolid())
	{
		double surface = static_cast<double>(static_cast<float>(by + 1) - LiquidTile::getHeight(level.getData(bx, by, bz)));
		if (y < surface)
			remove();
	}
}

HugeExplosionParticle::HugeExplosionParticle(Level &level, double x, double y, double z)
	: Particle(level, x, y, z, 0.0, 0.0, 0.0)
{
}

void HugeExplosionParticle::render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2)
{
	(void)t; (void)a; (void)xa; (void)ya; (void)za; (void)xa2; (void)za2;
}

void HugeExplosionParticle::tick()
{
	for (int_t i = 0; i < 6; ++i)
	{
		double px = x + (random.nextDouble() - random.nextDouble()) * 4.0;
		double py = y + (random.nextDouble() - random.nextDouble()) * 4.0;
		double pz = z + (random.nextDouble() - random.nextDouble()) * 4.0;
		level.addParticle(u"largeexplode", px, py, pz, static_cast<double>(static_cast<float>(life) / static_cast<float>(maxLife)), 0.0, 0.0);
	}
	life++;
	if (life == maxLife)
		remove();
}

LargeExplosionParticle::LargeExplosionParticle(Textures &textures, Level &level, double x, double y, double z, double progress)
	: Particle(level, x, y, z, 0.0, 0.0, 0.0), textures(textures)
{
	maxLife = 6 + random.nextInt(4);
	rCol = gCol = bCol = random.nextFloat() * 0.6f + 0.4f;
	scaleFactor = 1.0f - static_cast<float>(progress) * 0.5f;
}

void LargeExplosionParticle::render(Tesselator &t, float a, float xa, float ya, float za, float xa2, float za2)
{
	int_t frame = static_cast<int_t>((static_cast<float>(life) + a) * 15.0f / static_cast<float>(maxLife));
	if (frame > 15)
		return;
	textures.bind(textures.loadTexture(u"/misc/explosion.png"));
	float u0 = static_cast<float>(frame % 4) / 4.0f;
	float u1 = u0 + 0.24975f;
	float v0 = static_cast<float>(frame / 4) / 4.0f;
	float v1 = v0 + 0.24975f;
	float r = 2.0f * scaleFactor;
	float px = static_cast<float>(xo + (x - xo) * a - xOff);
	float py = static_cast<float>(yo + (y - yo) * a - yOff);
	float pz = static_cast<float>(zo + (z - zo) * a - zOff);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_LIGHTING);
	Lighting::turnOff();
	t.begin();
	t.color(rCol, gCol, bCol, 1.0f);
	t.normal(0.0f, 1.0f, 0.0f);
	t.vertexUV(px - xa * r - xa2 * r, py - ya * r, pz - za * r - za2 * r, u1, v1);
	t.vertexUV(px - xa * r + xa2 * r, py + ya * r, pz - za * r + za2 * r, u1, v0);
	t.vertexUV(px + xa * r + xa2 * r, py + ya * r, pz + za * r + za2 * r, u0, v0);
	t.vertexUV(px + xa * r - xa2 * r, py - ya * r, pz + za * r - za2 * r, u0, v1);
	t.end();
	glEnable(GL_LIGHTING);
}

void LargeExplosionParticle::tick()
{
	xo = x;
	yo = y;
	zo = z;
	life++;
	if (life == maxLife)
		remove();
}
