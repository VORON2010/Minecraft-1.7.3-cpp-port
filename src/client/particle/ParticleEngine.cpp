#include "client/particle/ParticleEngine.h"

#include "client/particle/Particle.h"
#include "client/particle/TerrainParticle.h"
#include "client/renderer/Tesselator.h"
#include "client/renderer/Textures.h"

#include "world/level/Level.h"
#include "world/level/tile/Tile.h"

#include "util/Mth.h"
#include "java/String.h"

#include "OpenGL.h"

ParticleEngine::ParticleEngine(Level *level, Textures *textures)
	: level(level), textures(textures), random(Random())
{
	
	for (int_t i = 0; i < 4; i++)
	{
		particles[i].clear();
	}
}

void ParticleEngine::add(std::unique_ptr<Particle> p)
{
	
	int_t t = p->getParticleTexture();
	if (t >= 0 && t < 4)
	{
		particles[t].push_back(std::move(p));
	}
}

void ParticleEngine::tick()
{
	
	for (int_t tt = 0; tt < 4; tt++)
	{
		for (int_t i = 0; i < (int_t)particles[tt].size(); i++)
		{
			particles[tt][i]->tick();
			if (particles[tt][i]->removed)
			{
				particles[tt].erase(particles[tt].begin() + i);
				i--;
			}
		}
	}
}

void ParticleEngine::render(Entity &player, float a)
{
	
	float xa = Mth::cos(player.yRot * Mth::PI / 180.0f);
	float za = Mth::sin(player.yRot * Mth::PI / 180.0f);
	float xa2 = -za * Mth::sin(player.xRot * Mth::PI / 180.0f);
	float za2 = xa * Mth::sin(player.xRot * Mth::PI / 180.0f);
	float ya = Mth::cos(player.xRot * Mth::PI / 180.0f);

	
	Particle::xOff = player.xOld + (player.x - player.xOld) * a;
	Particle::yOff = player.yOld + (player.y - player.yOld) * a;
	Particle::zOff = player.zOld + (player.z - player.zOld) * a;

	
	for (int_t tt = 0; tt < 3; tt++)
	{
		if (!particles[tt].empty())
		{
			int_t id = 0;
			if (tt == 0)
			{
				id = textures->loadTexture(u"/particles.png");
			}
			else if (tt == 1)
			{
				id = textures->loadTexture(u"/terrain.png");
			}
			else if (tt == 2)
			{
				id = textures->loadTexture(u"/gui/items.png");
			}

			glBindTexture(GL_TEXTURE_2D, id);
			Tesselator &t = Tesselator::instance;
			t.begin();

			for (auto &p : particles[tt])
			{
				p->render(t, a, xa, ya, za, xa2, za2);
			}

			t.end();
		}
	}
}

void ParticleEngine::renderLit(Entity &player, float a)
{
	
	int_t tt = 3;
	if (!particles[tt].empty())
	{
		Tesselator &t = Tesselator::instance;

		for (auto &p : particles[tt])
		{
			p->render(t, a, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		}
	}
}

void ParticleEngine::setLevel(Level *level)
{
	this->level = level;

	
	for (int_t tt = 0; tt < 4; tt++)
	{
		particles[tt].clear();
	}
}

void ParticleEngine::destroy(int_t x, int_t y, int_t z)
{
	if (level == nullptr)
		return;
	destroy(x, y, z, level->getTile(x, y, z), level->getData(x, y, z));
}

void ParticleEngine::destroy(int_t x, int_t y, int_t z, int_t tileId, int_t data)
{
	if (level == nullptr)
		return;

	
	if (tileId != 0 && tileId >= 0 && tileId < static_cast<int_t>(Tile::tiles.size()) && Tile::tiles[tileId] != nullptr)
	{
		Tile *tile = Tile::tiles[tileId];
		int_t SD = 4;

		for (int_t xx = 0; xx < SD; xx++)
		{
			for (int_t yy = 0; yy < SD; yy++)
			{
				for (int_t zz = 0; zz < SD; zz++)
				{
					double xp = x + (xx + 0.5) / SD;
					double yp = y + (yy + 0.5) / SD;
					double zp = z + (zz + 0.5) / SD;
					int_t face = random.nextInt(6);

					auto particle = std::make_unique<TerrainParticle>(*level, xp, yp, zp, xp - x - 0.5, yp - y - 0.5, zp - z - 0.5, tile, face, data);
					particle->init(x, y, z);
					add(std::move(particle));
				}
			}
		}
	}
}

void ParticleEngine::crack(int_t x, int_t y, int_t z, int_t face)
{
	if (level == nullptr)
		return;

	
	int_t tid = level->getTile(x, y, z);
	if (tid != 0 && Tile::tiles[tid] != nullptr)
	{
		Tile *tile = Tile::tiles[tid];
		int_t data = level->getData(x, y, z);
		float r = 0.1f;
		double xp = x + random.nextDouble() * (tile->xx1 - tile->xx0 - r * 2.0f) + r + tile->xx0;
		double yp = y + random.nextDouble() * (tile->yy1 - tile->yy0 - r * 2.0f) + r + tile->yy0;
		double zp = z + random.nextDouble() * (tile->zz1 - tile->zz0 - r * 2.0f) + r + tile->zz0;

		
		if (face == 0)
		{
			yp = y + tile->yy0 - r;
		}
		else if (face == 1)
		{
			yp = y + tile->yy1 + r;
		}
		else if (face == 2)
		{
			zp = z + tile->zz0 - r;
		}
		else if (face == 3)
		{
			zp = z + tile->zz1 + r;
		}
		else if (face == 4)
		{
			xp = x + tile->xx0 - r;
		}
		else if (face == 5)
		{
			xp = x + tile->xx1 + r;
		}

		auto particle = std::make_unique<TerrainParticle>(*level, xp, yp, zp, 0.0, 0.0, 0.0, tile, face, data);
		particle->init(x, y, z);
		particle->setPower(0.2f);
		particle->scale(0.6f);
		add(std::move(particle));
	}
}

jstring ParticleEngine::countParticles()
{
	
	int_t count = (int_t)particles[0].size() + (int_t)particles[1].size() + (int_t)particles[2].size();
	return String::toString(count);
}
