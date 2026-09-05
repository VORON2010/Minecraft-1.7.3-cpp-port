#include "world/level/levelgen/feature/HellFeatures.h"

#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/FireTile.h"
#include "world/level/tile/GlowStoneTile.h"

bool FireFeature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	for (int_t i = 0; i < 64; ++i)
	{
		int_t xx = x + random.nextInt(8);
		xx -= random.nextInt(8);
		int_t yy = y + random.nextInt(4);
		yy -= random.nextInt(4);
		int_t zz = z + random.nextInt(8);
		zz -= random.nextInt(8);
		if (level.isEmptyTile(xx, yy, zz) && level.getTile(xx, yy - 1, zz) == Tile::netherrack.id)
			level.setTile(xx, yy, zz, Tile::fire.id);
	}

	return true;
}

static bool placeGlowStone(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	if (!level.isEmptyTile(x, y, z))
		return false;
	if (level.getTile(x, y + 1, z) != Tile::netherrack.id)
		return false;

	level.setTile(x, y, z, Tile::glowstone.id);

	for (int_t i = 0; i < 1500; ++i)
	{
		int_t xx = x + random.nextInt(8);
		xx -= random.nextInt(8);
		int_t yy = y - random.nextInt(12);
		int_t zz = z + random.nextInt(8);
		zz -= random.nextInt(8);
		if (level.getTile(xx, yy, zz) != 0)
			continue;

		int_t neighbours = 0;
		if (level.getTile(xx - 1, yy, zz) == Tile::glowstone.id)
			neighbours++;
		if (level.getTile(xx + 1, yy, zz) == Tile::glowstone.id)
			neighbours++;
		if (level.getTile(xx, yy - 1, zz) == Tile::glowstone.id)
			neighbours++;
		if (level.getTile(xx, yy + 1, zz) == Tile::glowstone.id)
			neighbours++;
		if (level.getTile(xx, yy, zz - 1) == Tile::glowstone.id)
			neighbours++;
		if (level.getTile(xx, yy, zz + 1) == Tile::glowstone.id)
			neighbours++;

		if (neighbours == 1)
			level.setTile(xx, yy, zz, Tile::glowstone.id);
	}

	return true;
}

bool GlowStone1Feature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	return placeGlowStone(level, random, x, y, z);
}

bool GlowStone2Feature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	return placeGlowStone(level, random, x, y, z);
}

HellLavaFeature::HellLavaFeature(int_t liquidId) : liquidId(liquidId)
{
}

bool HellLavaFeature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	if (level.getTile(x, y + 1, z) != Tile::netherrack.id)
		return false;
	if (level.getTile(x, y, z) != 0 && level.getTile(x, y, z) != Tile::netherrack.id)
		return false;

	int_t rockNeighbours = 0;
	if (level.getTile(x - 1, y, z) == Tile::netherrack.id)
		rockNeighbours++;
	if (level.getTile(x + 1, y, z) == Tile::netherrack.id)
		rockNeighbours++;
	if (level.getTile(x, y, z - 1) == Tile::netherrack.id)
		rockNeighbours++;
	if (level.getTile(x, y, z + 1) == Tile::netherrack.id)
		rockNeighbours++;
	if (level.getTile(x, y - 1, z) == Tile::netherrack.id)
		rockNeighbours++;

	int_t airNeighbours = 0;
	if (level.isEmptyTile(x - 1, y, z))
		airNeighbours++;
	if (level.isEmptyTile(x + 1, y, z))
		airNeighbours++;
	if (level.isEmptyTile(x, y, z - 1))
		airNeighbours++;
	if (level.isEmptyTile(x, y, z + 1))
		airNeighbours++;
	if (level.isEmptyTile(x, y - 1, z))
		airNeighbours++;

	if (rockNeighbours == 4 && airNeighbours == 1)
	{
		level.setTile(x, y, z, liquidId);
		level.instaTick = true;
		Tile::tiles[liquidId]->tick(level, x, y, z, random);
		level.instaTick = false;
	}

	return true;
}
