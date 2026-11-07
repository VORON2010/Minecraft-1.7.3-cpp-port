#include "world/entity/item/EnderCrystal.h"

#include "util/Mth.h"
#include "world/level/Level.h"
#include "world/level/tile/FireTile.h"
#include "world/level/tile/Tile.h"

EnderCrystal::EnderCrystal(Level &level) : Entity(level)
{
	blocksBuilding = true;
	setSize(2.0f, 2.0f);
	heightOffset = bbHeight / 2.0f;
	life = 5;
	time = random.nextInt(100000);
	makeStepSound = false;
	dataWatcher.addObject(8, life);
}

EnderCrystal::EnderCrystal(Level &level, double x, double y, double z) : EnderCrystal(level)
{
	setPos(x, y, z);
}

void EnderCrystal::tick()
{
	xo = x;
	yo = y;
	zo = z;
	time++;
	dataWatcher.updateObject(8, life);
	int_t tx = Mth::floor(x);
	int_t ty = Mth::floor(y);
	int_t tz = Mth::floor(z);
	if (level.getTile(tx, ty, tz) != Tile::fire.id)
		level.setTile(tx, ty, tz, Tile::fire.id);
}

bool EnderCrystal::isPickable()
{
	return true;
}

bool EnderCrystal::hurt(Entity *source, int_t dmg)
{
	(void)source;
	(void)dmg;
	if (!removed && !level.isOnline)
	{
		life = 0;
		if (life <= 0)
		{
			remove();
			if (!level.isOnline)
				level.createExplosion(nullptr, x, y, z, 6.0f);
		}
	}
	return true;
}

void EnderCrystal::addAdditionalSaveData(CompoundTag &tag)
{
	(void)tag;
}

void EnderCrystal::readAdditionalSaveData(CompoundTag &tag)
{
	(void)tag;
}
