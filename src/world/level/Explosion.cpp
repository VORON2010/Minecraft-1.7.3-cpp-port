#include "world/level/Explosion.h"

#include <cmath>
#include "world/level/Level.h"
#include "world/entity/Entity.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/FireTile.h"
#include "world/phys/AABB.h"
#include "world/phys/Vec3.h"
#include "util/Mth.h"
#include "java/Random.h"

Explosion::Explosion(Level &level, Entity *exploder, double x, double y, double z, float size)
	: level(level), exploder(exploder), explosionX(x), explosionY(y), explosionZ(z), explosionSize(size)
{
}

void Explosion::doExplosionA()
{
	float size = explosionSize;
	constexpr int_t gridSize = 16;

	for (int_t i = 0; i < gridSize; i++)
	{
		for (int_t j = 0; j < gridSize; j++)
		{
			for (int_t k = 0; k < gridSize; k++)
			{
				if (i == 0 || i == gridSize - 1 || j == 0 || j == gridSize - 1 || k == 0 || k == gridSize - 1)
				{
					double dx = i / (gridSize - 1.0f) * 2.0f - 1.0f;
					double dy = j / (gridSize - 1.0f) * 2.0f - 1.0f;
					double dz = k / (gridSize - 1.0f) * 2.0f - 1.0f;
					double len = std::sqrt(dx * dx + dy * dy + dz * dz);
					dx /= len;
					dy /= len;
					dz /= len;
					float power = explosionSize * (0.7f + level.random.nextFloat() * 0.6f);
					double cx = explosionX;
					double cy = explosionY;
					double cz = explosionZ;

					for (float step = 0.3f; power > 0.0f; power -= step * 0.75f)
					{
						int_t bx = Mth::floor(cx);
						int_t by = Mth::floor(cy);
						int_t bz = Mth::floor(cz);
						int_t tileId = level.getTile(bx, by, bz);
						if (tileId > 0)
						{
							Tile *tile = Tile::tiles[tileId];
							if (tile != nullptr)
								power -= (tile->getExplosionResistance(exploder) + 0.3f) * step;
						}

						if (power > 0.0f)
						{
							destroyedBlockPositions.emplace(bx, by, bz);
						}

						cx += dx * step;
						cy += dy * step;
						cz += dz * step;
					}
				}
			}
		}
	}

	explosionSize *= 2.0f;
	double minX = Mth::floor(explosionX - explosionSize - 1.0);
	double minY = Mth::floor(explosionY - explosionSize - 1.0);
	double minZ = Mth::floor(explosionZ - explosionSize - 1.0);
	double maxX = Mth::floor(explosionX + explosionSize + 1.0);
	double maxY = Mth::floor(explosionY + explosionSize + 1.0);
	double maxZ = Mth::floor(explosionZ + explosionSize + 1.0);

	std::vector<std::shared_ptr<Entity>> entities;
	AABB queryBox(minX, minY, minZ, maxX, maxY, maxZ);
	for (const auto &entity : level.getAllEntities())
	{
		if (entity->bb.intersects(queryBox))
			entities.push_back(entity);
	}

	for (const auto &entity : entities)
	{
		if (entity.get() == exploder)
			continue;
		double dist = entity->distanceTo(explosionX, explosionY, explosionZ) / explosionSize;
		if (dist <= 1.0)
		{
			double dx = entity->x - explosionX;
			double dy = entity->y - explosionY;
			double dz = entity->z - explosionZ;
			double len = std::sqrt(dx * dx + dy * dy + dz * dz);
			if (len == 0.0) len = 1.0;
			dx /= len;
			dy /= len;
			dz /= len;
			float density = getBlockDensity(explosionX, explosionY, explosionZ, entity->bb);
			double exposure = (1.0 - dist) * density;
			int_t damage = static_cast<int_t>((exposure * exposure + exposure) / 2.0 * 8.0 * explosionSize + 1.0);
			entity->hurt(exploder, damage);
			entity->xd += dx * exposure;
			entity->yd += dy * exposure;
			entity->zd += dz * exposure;
		}
	}

	explosionSize = size;

	
	
	
	if (isFlaming)
	{
		std::vector<TilePos> blocks(destroyedBlockPositions.begin(), destroyedBlockPositions.end());
		for (int_t i = static_cast<int_t>(blocks.size()) - 1; i >= 0; i--)
		{
			const TilePos &pos = blocks[i];
			int_t tileId = level.getTile(pos.x, pos.y, pos.z);
			int_t belowId = level.getTile(pos.x, pos.y - 1, pos.z);
			if (tileId == 0 && belowId > 0 && Tile::tiles[belowId] != nullptr && Tile::tiles[belowId]->isSolidRender() && explosionRNG.nextInt(3) == 0)
			{
				level.setTile(pos.x, pos.y, pos.z, Tile::fire.id);
			}
		}
	}
}

void Explosion::doExplosionB(bool particles)
{
	level.playSoundEffect(explosionX, explosionY, explosionZ, u"random.explode",
		4.0f, (1.0f + (level.random.nextFloat() - level.random.nextFloat()) * 0.2f) * 0.7f);

	std::vector<TilePos> blocks(destroyedBlockPositions.begin(), destroyedBlockPositions.end());

	for (int_t i = static_cast<int_t>(blocks.size()) - 1; i >= 0; i--)
	{
		const TilePos &pos = blocks[i];
		int_t tileId = level.getTile(pos.x, pos.y, pos.z);

		if (particles)
		{
			double px = pos.x + level.random.nextFloat();
			double py = pos.y + level.random.nextFloat();
			double pz = pos.z + level.random.nextFloat();
			double dx = px - explosionX;
			double dy = py - explosionY;
			double dz = pz - explosionZ;
			double dist = std::sqrt(dx * dx + dy * dy + dz * dz);
			if (dist != 0.0)
			{
				dx /= dist;
				dy /= dist;
				dz /= dist;
			}
			double speed = 0.5 / (dist / explosionSize + 0.1);
			speed *= level.random.nextFloat() * level.random.nextFloat() + 0.3f;
			dx *= speed;
			dy *= speed;
			dz *= speed;
			level.addParticle(u"explode", (px + explosionX) / 2.0, (py + explosionY) / 2.0, (pz + explosionZ) / 2.0, dx, dy, dz);
			level.addParticle(u"smoke", px, py, pz, dx, dy, dz);
		}

		if (tileId > 0)
		{
			Tile *tile = Tile::tiles[tileId];
			if (tile != nullptr)
			{
				int_t data = level.getData(pos.x, pos.y, pos.z);
				tile->spawnResources(level, pos.x, pos.y, pos.z, data, 0.3f);
			}
			level.setTile(pos.x, pos.y, pos.z, 0);
			if (tile != nullptr)
				tile->onBlockDestroyedByExplosion(level, pos.x, pos.y, pos.z);
		}
	}
}

float Explosion::getBlockDensity(double x, double y, double z, const AABB &bb)
{
	Vec3 origin(x, y, z);
	double stepX = 1.0 / ((bb.x1 - bb.x0) * 2.0 + 1.0);
	double stepY = 1.0 / ((bb.y1 - bb.y0) * 2.0 + 1.0);
	double stepZ = 1.0 / ((bb.z1 - bb.z0) * 2.0 + 1.0);

	int visible = 0;
	int total = 0;

	for (double fx = 0.0; fx <= 1.0; fx += stepX)
	{
		for (double fy = 0.0; fy <= 1.0; fy += stepY)
		{
			for (double fz = 0.0; fz <= 1.0; fz += stepZ)
			{
				Vec3 point(
					bb.x0 + (bb.x1 - bb.x0) * fx,
					bb.y0 + (bb.y1 - bb.y0) * fy,
					bb.z0 + (bb.z1 - bb.z0) * fz
				);
				HitResult hit = level.clip(point, origin);
				if (hit.type == HitResult::Type::NONE)
					visible++;
				total++;
			}
		}
	}

	return total == 0 ? 0.0f : (float)visible / (float)total;
}
