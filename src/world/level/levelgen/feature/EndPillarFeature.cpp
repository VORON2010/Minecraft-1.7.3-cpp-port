#include "world/level/levelgen/feature/EndPillarFeature.h"

#include "world/entity/item/EnderCrystal.h"
#include "world/level/Level.h"
#include "world/level/tile/Tile.h"

bool EndPillarFeature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	if (!level.isEmptyTile(x, y, z) || level.getTile(x, y - 1, z) != tile)
		return false;
	int_t height = random.nextInt(32) + 6;
	int_t radius = random.nextInt(4) + 1;
	for (int_t xx = x - radius; xx <= x + radius; xx++)
	{
		for (int_t zz = z - radius; zz <= z + radius; zz++)
		{
			int_t dx = xx - x;
			int_t dz = zz - z;
			if (dx * dx + dz * dz <= radius * radius + 1 && level.getTile(xx, y - 1, zz) != tile)
				return false;
		}
	}
	for (int_t yy = y; yy < y + height && yy < 128; yy++)
	{
		for (int_t xx = x - radius; xx <= x + radius; xx++)
		{
			for (int_t zz = z - radius; zz <= z + radius; zz++)
			{
				int_t dx = xx - x;
				int_t dz = zz - z;
				if (dx * dx + dz * dz <= radius * radius + 1)
					level.setTile(xx, yy, zz, Tile::obsidian.id);
			}
		}
	}
	auto crystal = std::make_shared<EnderCrystal>(level);
	crystal->moveTo(x + 0.5f, y + height, z + 0.5f, random.nextFloat() * 360.0f, 0.0f);
	level.addEntity(crystal);
	level.setTile(x, y + height, z, Tile::bedrock.id);
	return true;
}
