#include "world/level/tile/DragonEggTile.h"

#include "world/entity/item/FallingTile.h"
#include "world/level/Level.h"
#include "world/level/tile/SandTile.h"

DragonEggTile::DragonEggTile(int_t id, int_t tex) : Tile(id, tex, Material::egg)
{
	// non-solid: lets light through and keeps neighbour faces
	updateCachedProperties();
}

void DragonEggTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	level.addToTickNextTick(x, y, z, id);
}

void DragonEggTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	(void)tile;
	level.addToTickNextTick(x, y, z, id);
}

void DragonEggTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	(void)random;
	checkSlide(level, x, y, z);
}

void DragonEggTile::checkSlide(Level &level, int_t x, int_t y, int_t z)
{
	if (SandTile::isFree(level, x, y - 1, z) && y >= 0)
	{
		int_t r = 32;
		if (!SandTile::fallInstantly && level.hasChunksAt(x - r, y - r, z - r, x + r, y + r, z + r))
		{
			level.addEntity(std::make_shared<FallingTile>(level, x + 0.5f, y + 0.5f, z + 0.5f, id));
		}
		else
		{
			level.setTile(x, y, z, 0);
			while (SandTile::isFree(level, x, y - 1, z) && y > 0)
				y--;
			if (y > 0)
				level.setTile(x, y, z, id);
		}
	}
}

bool DragonEggTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	(void)player;
	teleport(level, x, y, z);
	return true;
}

void DragonEggTile::attack(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	(void)player;
	teleport(level, x, y, z);
}

void DragonEggTile::teleport(Level &level, int_t x, int_t y, int_t z)
{
	if (level.getTile(x, y, z) != id || level.isOnline)
		return;
	for (int_t i = 0; i < 1000; i++)
	{
		int_t tx = x + level.random.nextInt(16) - level.random.nextInt(16);
		int_t ty = y + level.random.nextInt(8) - level.random.nextInt(8);
		int_t tz = z + level.random.nextInt(16) - level.random.nextInt(16);
		if (level.getTile(tx, ty, tz) != 0)
			continue;
		level.setTileAndData(tx, ty, tz, id, level.getData(x, y, z));
		level.setTile(x, y, z, 0);
		for (int_t p = 0; p < 128; p++)
		{
			double t = level.random.nextDouble();
			float vx = (level.random.nextFloat() - 0.5f) * 0.2f;
			float vy = (level.random.nextFloat() - 0.5f) * 0.2f;
			float vz = (level.random.nextFloat() - 0.5f) * 0.2f;
			double px = tx + (x - tx) * t + (level.random.nextDouble() - 0.5) * 1.0 + 0.5;
			double py = ty + (y - ty) * t + level.random.nextDouble() * 1.0 - 0.5;
			double pz = tz + (z - tz) * t + (level.random.nextDouble() - 0.5) * 1.0 + 0.5;
			level.addParticle(u"portal", px, py, pz, vx, vy, vz);
		}
		return;
	}
}

int_t DragonEggTile::getTickDelay()
{
	return 3;
}

bool DragonEggTile::isSolidRender()
{
	return false;
}

bool DragonEggTile::isCubeShaped()
{
	return false;
}
