#include "world/level/levelgen/feature/LakeFeature.h"

#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/StoneTile.h"
#include "world/level/tile/GrassTile.h"
#include "world/level/tile/DirtTile.h"
#include "world/level/material/Material.h"
#include "world/level/material/LiquidMaterial.h"

#include <cstring>

LakeFeature::LakeFeature(int_t blockId) : blockId(blockId)
{
}

bool LakeFeature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	x -= 8;

	for (z -= 8; y > 0 && level.isEmptyTile(x, y, z); --y)
	{
	}

	y -= 4;

	bool shape[2048];
	std::memset(shape, 0, sizeof(shape));

	int_t ellipsoidCount = random.nextInt(4) + 4;

	for (int_t i = 0; i < ellipsoidCount; ++i)
	{
		double rx = random.nextDouble() * 6.0 + 3.0;
		double ry = random.nextDouble() * 4.0 + 2.0;
		double rz = random.nextDouble() * 6.0 + 3.0;
		double cx = random.nextDouble() * (16.0 - rx - 2.0) + 1.0 + rx / 2.0;
		double cy = random.nextDouble() * (8.0 - ry - 4.0) + 2.0 + ry / 2.0;
		double cz = random.nextDouble() * (16.0 - rz - 2.0) + 1.0 + rz / 2.0;

		for (int_t sx = 1; sx < 15; ++sx)
		{
			for (int_t sz = 1; sz < 15; ++sz)
			{
				for (int_t sy = 1; sy < 7; ++sy)
				{
					double dx = (static_cast<double>(sx) - cx) / (rx / 2.0);
					double dy = (static_cast<double>(sy) - cy) / (ry / 2.0);
					double dz = (static_cast<double>(sz) - cz) / (rz / 2.0);
					double dist = dx * dx + dy * dy + dz * dz;
					if (dist < 1.0)
					{
						shape[(sx * 16 + sz) * 8 + sy] = true;
					}
				}
			}
		}
	}

	
	for (int_t sx = 0; sx < 16; ++sx)
	{
		for (int_t sz = 0; sz < 16; ++sz)
		{
			for (int_t sy = 0; sy < 8; ++sy)
			{
				bool border = !shape[(sx * 16 + sz) * 8 + sy] &&
					(sx < 15 && shape[((sx + 1) * 16 + sz) * 8 + sy] ||
					 sx > 0  && shape[((sx - 1) * 16 + sz) * 8 + sy] ||
					 sz < 15 && shape[(sx * 16 + sz + 1) * 8 + sy] ||
					 sz > 0  && shape[(sx * 16 + (sz - 1)) * 8 + sy] ||
					 sy < 7  && shape[(sx * 16 + sz) * 8 + sy + 1] ||
					 sy > 0  && shape[(sx * 16 + sz) * 8 + (sy - 1)]);

				if (border)
				{
					const Material &mat = level.getMaterial(x + sx, y + sy, z + sz);
					if (sy >= 4 && mat.isLiquid())
						return false;
					if (sy < 4 && !mat.isSolid() && level.getTile(x + sx, y + sy, z + sz) != blockId)
						return false;
				}
			}
		}
	}

	
	for (int_t sx = 0; sx < 16; ++sx)
	{
		for (int_t sz = 0; sz < 16; ++sz)
		{
			for (int_t sy = 0; sy < 8; ++sy)
			{
				if (shape[(sx * 16 + sz) * 8 + sy])
				{
					level.setTileNoUpdate(x + sx, y + sy, z + sz, sy >= 4 ? 0 : blockId);
				}
			}
		}
	}

	
	for (int_t sx = 0; sx < 16; ++sx)
	{
		for (int_t sz = 0; sz < 16; ++sz)
		{
			for (int_t sy = 4; sy < 8; ++sy)
			{
				if (shape[(sx * 16 + sz) * 8 + sy] &&
					level.getTile(x + sx, y + sy - 1, z + sz) == Tile::dirt.id &&
					level.getBrightness(LightLayer::Sky, x + sx, y + sy, z + sz) > 0)
				{
					level.setTileNoUpdate(x + sx, y + sy - 1, z + sz, Tile::grass.id);
				}
			}
		}
	}

	
	if (&Tile::tiles[blockId]->material == &Material::lava)
	{
		for (int_t sx = 0; sx < 16; ++sx)
		{
			for (int_t sz = 0; sz < 16; ++sz)
			{
				for (int_t sy = 0; sy < 8; ++sy)
				{
					bool border = !shape[(sx * 16 + sz) * 8 + sy] &&
						(sx < 15 && shape[((sx + 1) * 16 + sz) * 8 + sy] ||
						 sx > 0  && shape[((sx - 1) * 16 + sz) * 8 + sy] ||
						 sz < 15 && shape[(sx * 16 + sz + 1) * 8 + sy] ||
						 sz > 0  && shape[(sx * 16 + (sz - 1)) * 8 + sy] ||
						 sy < 7  && shape[(sx * 16 + sz) * 8 + sy + 1] ||
						 sy > 0  && shape[(sx * 16 + sz) * 8 + (sy - 1)]);

					if (border && (sy < 4 || random.nextInt(2) != 0) &&
						level.getMaterial(x + sx, y + sy, z + sz).isSolid())
					{
						level.setTileNoUpdate(x + sx, y + sy, z + sz, Tile::rock.id);
					}
				}
			}
		}
	}

	return true;
}
