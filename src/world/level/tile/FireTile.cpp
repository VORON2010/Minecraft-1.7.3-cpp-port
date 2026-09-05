#include "world/level/tile/FireTile.h"

#include <array>

#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/material/GasMaterial.h"
#include "world/level/tile/BookshelfTile.h"
#include "world/level/tile/ClothTile.h"
#include "world/level/tile/FenceTile.h"
#include "world/level/tile/LeafTile.h"
#include "world/level/tile/StairTile.h"
#include "world/level/tile/TallGrassTile.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/TNTTile.h"
#include "world/level/tile/TreeTile.h"
#include "world/level/tile/WoodTile.h"

#if __has_include("world/level/tile/PortalTile.h")
#include "world/level/tile/PortalTile.h"
#define HAS_PORTAL_TILE 1
#else
#define HAS_PORTAL_TILE 0
#endif

namespace
{
	constexpr int_t FIRE_TICK_DELAY = 40;

	struct FireOddsTables
	{
		std::array<int_t, 256> encouragement{};
		std::array<int_t, 256> flammability{};

		FireOddsTables()
		{
			auto setOdds = [this](int_t tileId, int_t encouragementChance, int_t flammabilityChance)
			{
				encouragement[tileId] = encouragementChance;
				flammability[tileId] = flammabilityChance;
			};

			setOdds(Tile::wood.id, 5, 20);
			setOdds(Tile::fence.id, 5, 20);
			setOdds(Tile::stairsWood.id, 5, 20);
			setOdds(Tile::treeTrunk.id, 5, 5);
			setOdds(Tile::leaves.id, 30, 60);
			setOdds(Tile::bookshelf.id, 30, 20);
			setOdds(Tile::tnt.id, 15, 100);
			setOdds(Tile::tallGrass.id, 60, 100);
			setOdds(Tile::wool.id, 30, 60);
		}
	};

	FireOddsTables &getFireOddsTables()
	{
		static FireOddsTables tables;
		return tables;
	}

	int_t getEncouragementOdds(int_t tileId)
	{
		if (tileId < 0 || tileId >= static_cast<int_t>(Tile::tiles.size()))
			return 0;
		return getFireOddsTables().encouragement[tileId];
	}

	int_t getFlammabilityOdds(int_t tileId)
	{
		if (tileId < 0 || tileId >= static_cast<int_t>(Tile::tiles.size()))
			return 0;
		return getFireOddsTables().flammability[tileId];
	}

	bool hasFlammableNeighbor(LevelSource &level, int_t x, int_t y, int_t z)
	{
		return FireTile::canBlockCatchFire(level, x + 1, y, z)
			|| FireTile::canBlockCatchFire(level, x - 1, y, z)
			|| FireTile::canBlockCatchFire(level, x, y - 1, z)
			|| FireTile::canBlockCatchFire(level, x, y + 1, z)
			|| FireTile::canBlockCatchFire(level, x, y, z - 1)
			|| FireTile::canBlockCatchFire(level, x, y, z + 1);
	}

	int_t getNeighborEncouragement(LevelSource &level, int_t x, int_t y, int_t z, int_t current)
	{
		int_t encouragement = getEncouragementOdds(level.getTile(x, y, z));
		return encouragement > current ? encouragement : current;
	}

	int_t getChanceOfNeighborsEncouragingFire(LevelSource &level, int_t x, int_t y, int_t z)
	{
		if (level.getTile(x, y, z) != 0)
			return 0;

		int_t encouragement = 0;
		encouragement = getNeighborEncouragement(level, x + 1, y, z, encouragement);
		encouragement = getNeighborEncouragement(level, x - 1, y, z, encouragement);
		encouragement = getNeighborEncouragement(level, x, y - 1, z, encouragement);
		encouragement = getNeighborEncouragement(level, x, y + 1, z, encouragement);
		encouragement = getNeighborEncouragement(level, x, y, z - 1, encouragement);
		encouragement = getNeighborEncouragement(level, x, y, z + 1, encouragement);
		return encouragement;
	}

	void tryToCatchBlockOnFire(FireTile &fire, Level &level, int_t x, int_t y, int_t z, int_t chance, Random &random, int_t fireAge)
	{
		int_t flammability = getFlammabilityOdds(level.getTile(x, y, z));
		if (random.nextInt(chance) < flammability)
		{
			bool isTnt = level.getTile(x, y, z) == Tile::tnt.id;
			if (random.nextInt(fireAge + 10) < 5 && !level.canBlockBeRainedOn(x, y, z))
			{
				int_t newAge = fireAge + random.nextInt(5) / 4;
				if (newAge > 15)
					newAge = 15;
				level.setTileAndData(x, y, z, fire.id, newAge);
			}
			else
			{
				level.setTile(x, y, z, 0);
			}

			if (isTnt)
			{
				Tile *tile = Tile::tiles[Tile::tnt.id];
				if (tile != nullptr)
					tile->playerDestroy(level, x, y, z, 1);
			}
		}
	}

#if HAS_PORTAL_TILE
	bool trySpawnPortal(Level &level, int_t x, int_t y, int_t z)
	{
		for (Tile *tile : Tile::tiles)
		{
			auto *portal = dynamic_cast<PortalTile *>(tile);
			if (portal != nullptr)
				return portal->trySpawnPortal(level, x, y, z);
		}
		return false;
	}
#else
	bool trySpawnPortal(Level &level, int_t x, int_t y, int_t z)
	{
		(void)level;
		(void)x;
		(void)y;
		(void)z;
		return false;
	}
#endif

	void addSmokeParticle(Level &level, double x, double y, double z)
	{
		level.addParticle(u"largesmoke", x, y, z, 0.0, 0.0, 0.0);
	}
}

FireTile::FireTile(int_t id, int_t tex) : Tile(id, tex, Material::fire)
{
	setDestroyTime(0.0f);
	setLightEmission(15);
	setSoundType(soundWoodFootstep);
	setTicking(true);
	updateCachedProperties();
}

Tile::Shape FireTile::getRenderShape()
{
	return SHAPE_FIRE;
}

AABB *FireTile::getAABB(Level &level, int_t x, int_t y, int_t z)
{
	(void)level;
	(void)x;
	(void)y;
	(void)z;
	return nullptr;
}

bool FireTile::isSolidRender()
{
	return false;
}

bool FireTile::isCubeShaped()
{
	return false;
}

int_t FireTile::getResourceCount(Random &random)
{
	(void)random;
	return 0;
}

int_t FireTile::getTickDelay()
{
	return FIRE_TICK_DELAY;
}

void FireTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	bool onNetherrack = level.getTile(x, y - 1, z) == Tile::netherrack.id;
	if (!mayPlace(level, x, y, z))
		level.setTile(x, y, z, 0);

	int_t fireAge = level.getData(x, y, z);
	if (fireAge < 15)
	{
		fireAge += random.nextInt(3) / 2;
		level.setData(x, y, z, fireAge);
	}

	level.scheduleBlockUpdate(x, y, z, id, getTickDelay());
	if (!onNetherrack
		&& level.isRaining()
		&& (level.canBlockBeRainedOn(x, y, z)
			|| level.canBlockBeRainedOn(x - 1, y, z)
			|| level.canBlockBeRainedOn(x + 1, y, z)
			|| level.canBlockBeRainedOn(x, y, z - 1)
			|| level.canBlockBeRainedOn(x, y, z + 1)))
	{
		level.setTile(x, y, z, 0);
		return;
	}

	if (!onNetherrack && !hasFlammableNeighbor(level, x, y, z))
	{
		if (!level.isBlockNormalCube(x, y - 1, z) || fireAge > 3)
			level.setTile(x, y, z, 0);
		return;
	}

	if (!onNetherrack && !canBlockCatchFire(level, x, y - 1, z) && fireAge == 15 && random.nextInt(4) == 0)
	{
		level.setTile(x, y, z, 0);
		return;
	}

	tryToCatchBlockOnFire(*this, level, x + 1, y, z, 300, random, fireAge);
	tryToCatchBlockOnFire(*this, level, x - 1, y, z, 300, random, fireAge);
	tryToCatchBlockOnFire(*this, level, x, y - 1, z, 250, random, fireAge);
	tryToCatchBlockOnFire(*this, level, x, y + 1, z, 250, random, fireAge);
	tryToCatchBlockOnFire(*this, level, x, y, z - 1, 300, random, fireAge);
	tryToCatchBlockOnFire(*this, level, x, y, z + 1, 300, random, fireAge);

	for (int_t xx = x - 1; xx <= x + 1; ++xx)
	{
		for (int_t zz = z - 1; zz <= z + 1; ++zz)
		{
			for (int_t yy = y - 1; yy <= y + 4; ++yy)
			{
				if (xx == x && yy == y && zz == z)
					continue;

				int_t chance = 100;
				if (yy > y + 1)
					chance += (yy - (y + 1)) * 100;

				int_t encouragement = getChanceOfNeighborsEncouragingFire(level, xx, yy, zz);
				if (encouragement <= 0)
					continue;

				int_t spreadChance = (encouragement + 40) / (fireAge + 30);
				if (spreadChance <= 0 || random.nextInt(chance) > spreadChance)
					continue;

				if (level.isRaining()
					&& (level.canBlockBeRainedOn(xx, yy, zz)
						|| level.canBlockBeRainedOn(xx - 1, yy, zz)
						|| level.canBlockBeRainedOn(xx + 1, yy, zz)
						|| level.canBlockBeRainedOn(xx, yy, zz - 1)
						|| level.canBlockBeRainedOn(xx, yy, zz + 1)))
					continue;

				int_t newAge = fireAge + random.nextInt(5) / 4;
				if (newAge > 15)
					newAge = 15;
				level.setTileAndData(xx, yy, zz, id, newAge);
			}
		}
	}
}

void FireTile::animateTick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	if (random.nextInt(24) == 0)
	{
		level.playSoundEffect((double)x + 0.5, (double)y + 0.5, (double)z + 0.5,
			u"fire.fire", 1.0f + random.nextFloat(), random.nextFloat() * 0.7f + 0.3f);
	}

	if (!level.isBlockNormalCube(x, y - 1, z) && !canBlockCatchFire(level, x, y - 1, z))
	{
		if (canBlockCatchFire(level, x - 1, y, z))
		{
			for (int_t i = 0; i < 2; ++i)
				addSmokeParticle(level, (double)x + random.nextFloat() * 0.1, (double)y + random.nextFloat(), (double)z + random.nextFloat());
		}
		if (canBlockCatchFire(level, x + 1, y, z))
		{
			for (int_t i = 0; i < 2; ++i)
				addSmokeParticle(level, (double)(x + 1) - random.nextFloat() * 0.1, (double)y + random.nextFloat(), (double)z + random.nextFloat());
		}
		if (canBlockCatchFire(level, x, y, z - 1))
		{
			for (int_t i = 0; i < 2; ++i)
				addSmokeParticle(level, (double)x + random.nextFloat(), (double)y + random.nextFloat(), (double)z + random.nextFloat() * 0.1);
		}
		if (canBlockCatchFire(level, x, y, z + 1))
		{
			for (int_t i = 0; i < 2; ++i)
				addSmokeParticle(level, (double)x + random.nextFloat(), (double)y + random.nextFloat(), (double)(z + 1) - random.nextFloat() * 0.1);
		}
		if (canBlockCatchFire(level, x, y + 1, z))
		{
			for (int_t i = 0; i < 2; ++i)
				addSmokeParticle(level, (double)x + random.nextFloat(), (double)(y + 1) - random.nextFloat() * 0.1, (double)z + random.nextFloat());
		}
		return;
	}

	for (int_t i = 0; i < 3; ++i)
	{
		addSmokeParticle(level,
			(double)x + random.nextFloat(),
			(double)y + random.nextFloat() * 0.5 + 0.5,
			(double)z + random.nextFloat());
	}
}

bool FireTile::mayPick()
{
	return false;
}

bool FireTile::mayPlace(Level &level, int_t x, int_t y, int_t z)
{
	return level.isBlockNormalCube(x, y - 1, z) || hasFlammableNeighbor(level, x, y, z);
}

void FireTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	(void)tile;
	if (!level.isBlockNormalCube(x, y - 1, z) && !hasFlammableNeighbor(level, x, y, z))
		level.setTile(x, y, z, 0);
}

void FireTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	if (level.getTile(x, y - 1, z) == Tile::obsidian.id && trySpawnPortal(level, x, y, z))
		return;

	if (!level.isBlockNormalCube(x, y - 1, z) && !hasFlammableNeighbor(level, x, y, z))
	{
		level.setTile(x, y, z, 0);
		return;
	}

	level.scheduleBlockUpdate(x, y, z, id, getTickDelay());
}

bool FireTile::canBlockCatchFire(LevelSource &level, int_t x, int_t y, int_t z)
{
	return getEncouragementOdds(level.getTile(x, y, z)) > 0;
}
