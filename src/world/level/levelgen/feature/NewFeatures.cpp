// World generation features added in b1.8 / 1.0, ported from the 1.0 sources.
#include "world/level/levelgen/feature/BlockPatchFeature.h"
#include "world/level/levelgen/feature/HugeMushroomFeature.h"
#include "world/level/levelgen/feature/SwampTreeFeature.h"
#include "world/level/levelgen/feature/WaterlilyFeature.h"

#include <cstdlib>

#include "world/level/Level.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/level/material/Material.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/DirtTile.h"
#include "world/level/tile/GrassTile.h"
#include "world/level/tile/HugeMushroomTile.h"
#include "world/level/tile/LeafTile.h"
#include "world/level/tile/LiquidTile.h"
#include "world/level/tile/MushroomTile.h"
#include "world/level/tile/MyceliumTile.h"
#include "world/level/tile/TreeTile.h"
#include "world/level/tile/VineTile.h"
#include "world/level/tile/WaterlilyTile.h"

namespace
{
	bool isWater(Level &level, int_t x, int_t y, int_t z)
	{
		return &level.getMaterial(x, y, z) == static_cast<const Material *>(&Material::water);
	}
}

BlockPatchFeature::BlockPatchFeature(int_t radius, int_t block) : block(block), radius(radius)
{
}

bool BlockPatchFeature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	if (!isWater(level, x, y, z))
		return false;

	int_t r = random.nextInt(radius - 2) + 2;
	int_t yr = 2;
	for (int_t xx = x - r; xx <= x + r; ++xx)
	{
		for (int_t zz = z - r; zz <= z + r; ++zz)
		{
			int_t dx = xx - x;
			int_t dz = zz - z;
			if (dx * dx + dz * dz > r * r)
				continue;
			for (int_t yy = y - yr; yy <= y + yr; ++yy)
			{
				int_t t = level.getTile(xx, yy, zz);
				if (t == Tile::dirt.id || t == Tile::grass.id)
					level.setTileNoUpdate(xx, yy, zz, block);
			}
		}
	}
	return true;
}

bool WaterlilyFeature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	for (int_t i = 0; i < 10; ++i)
	{
		int_t xx = x + random.nextInt(8) - random.nextInt(8);
		int_t yy = y + random.nextInt(4) - random.nextInt(4);
		int_t zz = z + random.nextInt(8) - random.nextInt(8);
		if (level.isEmptyTile(xx, yy, zz) && Tile::waterlily.canStay(level, xx, yy, zz))
			level.setTileNoUpdate(xx, yy, zz, Tile::waterlily.id);
	}
	return true;
}

HugeMushroomFeature::HugeMushroomFeature(int_t forcedType) : forcedType(forcedType)
{
}

bool HugeMushroomFeature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	int_t type = random.nextInt(2);
	if (forcedType >= 0)
		type = forcedType;

	int_t height = random.nextInt(3) + 4;
	bool free = true;
	if (y < 1 || y + height + 1 > Level::DEPTH)
		return false;

	for (int_t yy = y; yy <= y + 1 + height; ++yy)
	{
		int_t r = 3;
		if (yy == y)
			r = 0;
		for (int_t xx = x - r; xx <= x + r && free; ++xx)
		{
			for (int_t zz = z - r; zz <= z + r && free; ++zz)
			{
				if (yy >= 0 && yy < Level::DEPTH)
				{
					int_t t = level.getTile(xx, yy, zz);
					if (t != 0 && t != Tile::leaves.id)
						free = false;
				}
				else
				{
					free = false;
				}
			}
		}
	}

	if (!free)
		return false;

	int_t below = level.getTile(x, y - 1, z);
	if (below != Tile::dirt.id && below != Tile::grass.id && below != Tile::mycelium.id)
		return false;
	if (!Tile::brownMushroom.mayPlace(level, x, y, z))
		return false;

	level.setTileNoUpdate(x, y - 1, z, Tile::dirt.id);
	int_t low = y + height;
	if (type == 1)
		low = y + height - 3;

	for (int_t yy = low; yy <= y + height; ++yy)
	{
		int_t r = 1;
		if (yy < y + height)
			r++;
		if (type == 0)
			r = 3;

		for (int_t xx = x - r; xx <= x + r; ++xx)
		{
			for (int_t zz = z - r; zz <= z + r; ++zz)
			{
				int_t data = 5;
				if (xx == x - r) data--;
				if (xx == x + r) data++;
				if (zz == z - r) data -= 3;
				if (zz == z + r) data += 3;

				if (type == 0 || yy < y + height)
				{
					if ((xx == x - r || xx == x + r) && (zz == z - r || zz == z + r))
						continue;
					if (xx == x - (r - 1) && zz == z - r) data = 1;
					if (xx == x - r && zz == z - (r - 1)) data = 1;
					if (xx == x + (r - 1) && zz == z - r) data = 3;
					if (xx == x + r && zz == z - (r - 1)) data = 3;
					if (xx == x - (r - 1) && zz == z + r) data = 7;
					if (xx == x - r && zz == z + (r - 1)) data = 7;
					if (xx == x + (r - 1) && zz == z + r) data = 9;
					if (xx == x + r && zz == z + (r - 1)) data = 9;
				}

				if (data == 5 && yy < y + height)
					data = 0;

				if ((data != 0 || y >= y + height - 1) && !Tile::solid[level.getTile(xx, yy, zz)])
					level.setTileAndDataNoUpdate(xx, yy, zz, Tile::hugeMushroomBrown.id + type, data);
			}
		}
	}

	for (int_t i = 0; i < height; ++i)
	{
		int_t t = level.getTile(x, y + i, z);
		if (!Tile::solid[t])
			level.setTileAndDataNoUpdate(x, y + i, z, Tile::hugeMushroomBrown.id + type, 10);
	}
	return true;
}

bool SwampTreeFeature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	int_t height = random.nextInt(4) + 5;
	while (isWater(level, x, y - 1, z))
		y--;

	bool free = true;
	if (y < 1 || y + height + 1 > Level::DEPTH)
		return false;

	for (int_t yy = y; yy <= y + 1 + height; ++yy)
	{
		int_t r = 1;
		if (yy == y)
			r = 0;
		if (yy >= y + 1 + height - 2)
			r = 3;
		for (int_t xx = x - r; xx <= x + r && free; ++xx)
		{
			for (int_t zz = z - r; zz <= z + r && free; ++zz)
			{
				if (yy >= 0 && yy < Level::DEPTH)
				{
					int_t t = level.getTile(xx, yy, zz);
					if (t != 0 && t != Tile::leaves.id)
					{
						if (t != Tile::calmWater.id && t != Tile::water.id)
							free = false;
						else if (yy > y)
							free = false;
					}
				}
				else
				{
					free = false;
				}
			}
		}
	}

	if (!free)
		return false;

	int_t below = level.getTile(x, y - 1, z);
	if ((below != Tile::grass.id && below != Tile::dirt.id) || y >= Level::DEPTH - height - 1)
		return false;

	level.setTileNoUpdate(x, y - 1, z, Tile::dirt.id);

	for (int_t yy = y - 3 + height; yy <= y + height; ++yy)
	{
		int_t dy = yy - (y + height);
		int_t r = 2 - dy / 2;
		for (int_t xx = x - r; xx <= x + r; ++xx)
		{
			int_t dx = xx - x;
			for (int_t zz = z - r; zz <= z + r; ++zz)
			{
				int_t dz = zz - z;
				if ((std::abs(dx) != r || std::abs(dz) != r || (random.nextInt(2) != 0 && dy != 0)) && !Tile::solid[level.getTile(xx, yy, zz)])
					level.setTileNoUpdate(xx, yy, zz, Tile::leaves.id);
			}
		}
	}

	for (int_t i = 0; i < height; ++i)
	{
		int_t t = level.getTile(x, y + i, z);
		if (t == 0 || t == Tile::leaves.id || t == Tile::water.id || t == Tile::calmWater.id)
			level.setTileNoUpdate(x, y + i, z, Tile::treeTrunk.id);
	}

	for (int_t yy = y - 3 + height; yy <= y + height; ++yy)
	{
		int_t dy = yy - (y + height);
		int_t r = 2 - dy / 2;
		for (int_t xx = x - r; xx <= x + r; ++xx)
		{
			for (int_t zz = z - r; zz <= z + r; ++zz)
			{
				if (level.getTile(xx, yy, zz) != Tile::leaves.id)
					continue;
				if (random.nextInt(4) == 0 && level.getTile(xx - 1, yy, zz) == 0)
					addVine(level, xx - 1, yy, zz, 8);
				if (random.nextInt(4) == 0 && level.getTile(xx + 1, yy, zz) == 0)
					addVine(level, xx + 1, yy, zz, 2);
				if (random.nextInt(4) == 0 && level.getTile(xx, yy, zz - 1) == 0)
					addVine(level, xx, yy, zz - 1, 1);
				if (random.nextInt(4) == 0 && level.getTile(xx, yy, zz + 1) == 0)
					addVine(level, xx, yy, zz + 1, 4);
			}
		}
	}
	return true;
}

void SwampTreeFeature::addVine(Level &level, int_t x, int_t y, int_t z, int_t data)
{
	level.setTileAndData(x, y, z, Tile::vine.id, data);
	for (int_t left = 4; level.getTile(x, --y, z) == 0 && left > 0; --left)
		level.setTileAndData(x, y, z, Tile::vine.id, data);
}
