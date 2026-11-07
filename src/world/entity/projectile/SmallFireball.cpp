#include "world/entity/projectile/SmallFireball.h"

#include "world/entity/Mob.h"
#include "world/level/Level.h"
#include "world/level/tile/FireTile.h"
#include "world/level/tile/Tile.h"
#include "world/phys/HitResult.h"

SmallFireball::SmallFireball(Level &level) : EntityFireball(level)
{
	setSize(0.3125f, 0.3125f);
}

SmallFireball::SmallFireball(Level &level, Mob &owner, double xPower, double yPower, double zPower)
	: EntityFireball(level, owner, xPower, yPower, zPower)
{
	setSize(0.3125f, 0.3125f);
}

void SmallFireball::onHit(HitResult &hit)
{
	if (!level.isOnline)
	{
		if (hit.entity != nullptr)
		{
			std::shared_ptr<Entity> shooter = owner.lock();
			if (!hit.entity->fireImmune && hit.entity->hurt(shooter.get(), 5))
				hit.entity->onFire = std::max<int_t>(hit.entity->onFire, 5 * 20);
		}
		else
		{
			int_t tx = hit.x;
			int_t ty = hit.y;
			int_t tz = hit.z;
			switch (hit.f)
			{
			case Facing::DOWN: ty--; break;
			case Facing::UP: ty++; break;
			case Facing::NORTH: tz--; break;
			case Facing::SOUTH: tz++; break;
			case Facing::WEST: tx--; break;
			case Facing::EAST: tx++; break;
			default: break;
			}
			if (level.isEmptyTile(tx, ty, tz))
				level.setTile(tx, ty, tz, Tile::fire.id);
		}
		remove();
	}
}

bool SmallFireball::isPickable()
{
	return false;
}

bool SmallFireball::hurt(Entity *source, int_t dmg)
{
	(void)source;
	(void)dmg;
	return false;
}
