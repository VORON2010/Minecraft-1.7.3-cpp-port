#include "world/level/tile/entity/MobSpawnerTileEntity.h"

#include <typeinfo>

#include "world/entity/EntityIO.h"
#include "world/entity/Mob.h"
#include "world/level/Level.h"
#include "world/phys/AABB.h"

MobSpawnerTileEntity::MobSpawnerTileEntity()
{
	spawnDelay = 20;
}

bool MobSpawnerTileEntity::isNearPlayer() const
{
	if (level == nullptr)
		return false;
	for (const auto &player : level->players)
	{
		double dx = player->x - (x + 0.5);
		double dy = player->y - (y + 0.5);
		double dz = player->z - (z + 0.5);
		if (dx * dx + dy * dy + dz * dz < 16.0 * 16.0)
			return true;
	}
	return false;
}

void MobSpawnerTileEntity::tick()
{
	if (level == nullptr)
		return;

	oSpin = spin;
	if (!isNearPlayer())
		return;

	double px = x + level->random.nextFloat();
	double py = y + level->random.nextFloat();
	double pz = z + level->random.nextFloat();
	level->addParticle(u"smoke", px, py, pz, 0.0, 0.0, 0.0);
	level->addParticle(u"flame", px, py, pz, 0.0, 0.0, 0.0);

	spin += 1000.0 / (spawnDelay + 200.0);
	while (spin > 360.0)
	{
		spin -= 360.0;
		oSpin -= 360.0;
	}

	if (spawnDelay == -1)
		resetDelay();

	if (spawnDelay > 0)
	{
		spawnDelay--;
		return;
	}

	for (int_t attempt = 0; attempt < 4; attempt++)
	{
		std::shared_ptr<Entity> entity = EntityIO::newEntity(entityId, *level);
		Mob *mob = dynamic_cast<Mob *>(entity.get());
		if (mob == nullptr)
			return;

		
		AABB box(x, y, z, x + 1, y + 1, z + 1);
		AABB *grown = box.grow(8.0, 4.0, 8.0);
		const auto &nearby = level->getEntities(nullptr, *grown);
		int_t sameType = 0;
		for (const auto &other : nearby)
		{
			if (typeid(*other) == typeid(*mob))
				sameType++;
		}
		if (sameType >= 6)
		{
			resetDelay();
			return;
		}

		double sx = x + (level->random.nextDouble() - level->random.nextDouble()) * 4.0;
		double sy = y + level->random.nextInt(3) - 1;
		double sz = z + (level->random.nextDouble() - level->random.nextDouble()) * 4.0;
		mob->moveTo(sx, sy, sz, level->random.nextFloat() * 360.0f, 0.0f);
		if (mob->canSpawn())
		{
			level->addEntity(entity);

			for (int_t i = 0; i < 20; i++)
			{
				double sparkX = x + 0.5 + (level->random.nextFloat() - 0.5) * 2.0;
				double sparkY = y + 0.5 + (level->random.nextFloat() - 0.5) * 2.0;
				double sparkZ = z + 0.5 + (level->random.nextFloat() - 0.5) * 2.0;
				level->addParticle(u"smoke", sparkX, sparkY, sparkZ, 0.0, 0.0, 0.0);
				level->addParticle(u"flame", sparkX, sparkY, sparkZ, 0.0, 0.0, 0.0);
			}

			mob->spawnAnim();
			resetDelay();
		}
	}
}

void MobSpawnerTileEntity::resetDelay()
{
	if (level == nullptr)
		return;
	spawnDelay = 200 + level->random.nextInt(600);
}

void MobSpawnerTileEntity::load(CompoundTag &tag)
{
	TileEntity::load(tag);
	entityId = tag.getString(u"EntityId");
	spawnDelay = tag.getShort(u"Delay");
}

void MobSpawnerTileEntity::save(CompoundTag &tag)
{
	TileEntity::save(tag);
	tag.putString(u"EntityId", entityId);
	tag.putShort(u"Delay", static_cast<short_t>(spawnDelay));
}
