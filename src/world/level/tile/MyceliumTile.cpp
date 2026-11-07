#include "world/level/tile/MyceliumTile.h"

#include "world/level/Level.h"
#include "world/level/LevelSource.h"
#include "world/level/material/Material.h"
#include "world/level/tile/DirtTile.h"

MyceliumTile::MyceliumTile(int_t id) : Tile(id, 77, Material::grassMaterial)
{
	setTicking(true);
	updateCachedProperties();
}

int_t MyceliumTile::getTexture(LevelSource &level, int_t x, int_t y, int_t z, Facing face)
{
	if (face == Facing::UP)
		return 78;
	if (face == Facing::DOWN)
		return 2;
	const Material &above = level.getMaterial(x, y + 1, z);
	return &above == &Material::snow() || &above == &Material::builtSnow ? 68 : 77;
}

int_t MyceliumTile::getTexture(Facing face, int_t data)
{
	(void)data;
	if (face == Facing::UP)
		return 78;
	return face == Facing::DOWN ? 2 : 77;
}

void MyceliumTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	if (level.isOnline)
		return;
	if (level.getRawBrightness(x, y + 1, z) < 4 && Tile::lightBlock[level.getTile(x, y + 1, z)] > 2)
	{
		level.setTile(x, y, z, Tile::dirt.id);
	}
	else if (level.getRawBrightness(x, y + 1, z) >= 9)
	{
		for (int_t i = 0; i < 4; ++i)
		{
			int_t tx = x + random.nextInt(3) - 1;
			int_t ty = y + random.nextInt(5) - 3;
			int_t tz = z + random.nextInt(3) - 1;
			int_t above = level.getTile(tx, ty + 1, tz);
			if (level.getTile(tx, ty, tz) == Tile::dirt.id && level.getRawBrightness(tx, ty + 1, tz) >= 4 && Tile::lightBlock[above] <= 2)
				level.setTile(tx, ty, tz, id);
		}
	}
}

void MyceliumTile::animateTick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	Tile::animateTick(level, x, y, z, random);
	if (random.nextInt(10) == 0)
		level.addParticle(u"townaura", x + random.nextFloat(), y + 1.1f, z + random.nextFloat(), 0.0, 0.0, 0.0);
}

int_t MyceliumTile::getResource(int_t data, Random &random)
{
	(void)data;
	(void)random;
	return Tile::dirt.id;
}
