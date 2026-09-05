#include "world/level/tile/GrassTile.h"

#include "world/level/tile/DirtTile.h"
#include "world/level/Level.h"

#include "world/level/LevelSource.h"
#include "world/level/material/Material.h"
#include "world/level/GrassColor.h"

GrassTile::GrassTile(int_t id) : Tile(id, Material::grassMaterial)
{
	tex = 3;
	setTicking(true);
}

int_t GrassTile::getTexture(LevelSource &level, int_t x, int_t y, int_t z, Facing face)
{
	if (face == Facing::UP)
		return 0;
	if (face == Facing::DOWN)
		return 2;

	return &level.getMaterial(x, y + 1, z) == &Material::snow() ? 68 : 3;
}

int_t GrassTile::getTexture(Facing face, int_t data)
{
	(void)face;
	(void)data;
	return 3;
}

int_t GrassTile::getColor(LevelSource &level, int_t x, int_t y, int_t z)
{
	level.getBiomeSource().getBiomeBlock(x, z, 1, 1);
	double temperature = level.getBiomeSource().temperatures[0];
	double downfall = level.getBiomeSource().downfalls[0];
	return GrassColor::get(temperature, downfall);
}

int_t GrassTile::getItemColor(int_t data)
{
	return 0xFFFFFF;
}

void GrassTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	if (level.isOnline)
		return;
	if (level.getRawBrightness(x, y + 1, z) < 4 && Tile::lightBlock[level.getTile(x, y + 1, z)] > 2)
	{
		if (random.nextInt(4) != 0)
			return;
		level.setTile(x, y, z, Tile::dirt.id);
	}
	else if (level.getRawBrightness(x, y + 1, z) >= 9)
	{
		int_t targetX = x + random.nextInt(3) - 1;
		int_t targetY = y + random.nextInt(5) - 3;
		int_t targetZ = z + random.nextInt(3) - 1;
		int_t above = level.getTile(targetX, targetY + 1, targetZ);
		if (level.getTile(targetX, targetY, targetZ) == Tile::dirt.id
			&& level.getRawBrightness(targetX, targetY + 1, targetZ) >= 4
			&& Tile::lightBlock[above] <= 2)
			level.setTile(targetX, targetY, targetZ, Tile::grass.id);
	}
}

int_t GrassTile::getResource(int_t data, Random &random)
{
	return Tile::dirt.id;
}
