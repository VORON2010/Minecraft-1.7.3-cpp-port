#include "world/entity/EntityLightningBolt.h"

#include "world/level/tile/FireTile.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "util/Mth.h"

EntityLightningBolt::EntityLightningBolt(Level &level) : Entity(level)
{
	randomSeed = random.nextLong();
	flashes = random.nextInt(3) + 1;
}

EntityLightningBolt::EntityLightningBolt(Level &level, double x, double y, double z) : EntityLightningBolt(level)
{
	moveTo(x, y, z, 0.0f, 0.0f);
	life = 2;
	if (level.difficulty >= 2 && level.hasChunksAt(Mth::floor(x), Mth::floor(y), Mth::floor(z), 10))
	{
		int_t tx = Mth::floor(x);
		int_t ty = Mth::floor(y);
		int_t tz = Mth::floor(z);
		if (level.getTile(tx, ty, tz) == 0 && Tile::fire.mayPlace(level, tx, ty, tz))
			level.setTile(tx, ty, tz, Tile::fire.id);
		for (int_t i = 0; i < 4; ++i)
		{
			tx = Mth::floor(x) + random.nextInt(3) - 1;
			ty = Mth::floor(y) + random.nextInt(3) - 1;
			tz = Mth::floor(z) + random.nextInt(3) - 1;
			if (level.getTile(tx, ty, tz) == 0 && Tile::fire.mayPlace(level, tx, ty, tz))
				level.setTile(tx, ty, tz, Tile::fire.id);
		}
	}
}

void EntityLightningBolt::tick()
{
	Entity::tick();
	if (life == 2)
	{
		level.playSoundEffect(x, y, z, u"ambient.weather.thunder", 10000.0f, 0.8f + random.nextFloat() * 0.2f);
		level.playSoundEffect(x, y, z, u"random.explode", 2.0f, 0.5f + random.nextFloat() * 0.2f);
	}

	--life;
	if (life < 0)
	{
		if (flashes == 0)
		{
			remove();
		}
		else if (life < -random.nextInt(10))
		{
			--flashes;
			life = 1;
			randomSeed = random.nextLong();
			if (level.hasChunksAt(Mth::floor(x), Mth::floor(y), Mth::floor(z), 10))
			{
				int_t tx = Mth::floor(x);
				int_t ty = Mth::floor(y);
				int_t tz = Mth::floor(z);
				if (level.getTile(tx, ty, tz) == 0 && Tile::fire.mayPlace(level, tx, ty, tz))
					level.setTile(tx, ty, tz, Tile::fire.id);
			}
		}
	}

	if (life >= 0)
	{
		AABB strikeBox(x - 3.0, y - 3.0, z - 3.0, x + 3.0, y + 9.0, z + 3.0);
		const auto &targets = level.getEntities(this, strikeBox);
		for (const auto &target : targets)
		{
			if (target != nullptr)
				target->onStruckByLightning(*this);
		}
		level.lastLightningBolt = 2;
	}
}

bool EntityLightningBolt::shouldRenderAtSqrDistance(double distance)
{
	(void)distance;
	return life >= 0;
}

void EntityLightningBolt::readAdditionalSaveData(CompoundTag &tag)
{
	(void)tag;
}

void EntityLightningBolt::addAdditionalSaveData(CompoundTag &tag)
{
	(void)tag;
}
