#include "world/level/levelgen/feature/ClayFeature.h"

#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/SandTile.h"
#include "world/level/material/Material.h"
#include "world/level/material/LiquidMaterial.h"

#include "util/Mth.h"
ClayFeature::ClayFeature(int_t count) : count(count)
{
}

bool ClayFeature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	if (&level.getMaterial(x, y, z) != &Material::water)
		return false;

	float var6 = random.nextFloat() * Mth::PI;
	double var7 = (double)((float)(x + 8) + Mth::sin(var6) * (float)count / 8.0f);
	double var9 = (double)((float)(x + 8) - Mth::sin(var6) * (float)count / 8.0f);
	double var11 = (double)((float)(z + 8) + Mth::cos(var6) * (float)count / 8.0f);
	double var13 = (double)((float)(z + 8) - Mth::cos(var6) * (float)count / 8.0f);
	double var15 = (double)(y + random.nextInt(3) + 2);
	double var17 = (double)(y + random.nextInt(3) + 2);

	for (int_t i = 0; i <= count; ++i)
	{
		double var20 = var7 + (var9 - var7) * (double)i / (double)count;
		double var22 = var15 + (var17 - var15) * (double)i / (double)count;
		double var24 = var11 + (var13 - var11) * (double)i / (double)count;
		double var26 = random.nextDouble() * (double)count / 16.0;
		double var28 = (double)(Mth::sin((float)i * Mth::PI / (float)count) + 1.0f) * var26 + 1.0;
		double var30 = (double)(Mth::sin((float)i * Mth::PI / (float)count) + 1.0f) * var26 + 1.0;
		int_t var32 = Mth::floor(var20 - var28 / 2.0);
		int_t var33 = Mth::floor(var20 + var28 / 2.0);
		int_t var34 = Mth::floor(var22 - var30 / 2.0);
		int_t var35 = Mth::floor(var22 + var30 / 2.0);
		int_t var36 = Mth::floor(var24 - var28 / 2.0);
		int_t var37 = Mth::floor(var24 + var28 / 2.0);

		for (int_t bx = var32; bx <= var33; ++bx)
		{
			for (int_t by = var34; by <= var35; ++by)
			{
				for (int_t bz = var36; bz <= var37; ++bz)
				{
					double var41 = ((double)bx + 0.5 - var20) / (var28 / 2.0);
					double var43 = ((double)by + 0.5 - var22) / (var30 / 2.0);
					double var45 = ((double)bz + 0.5 - var24) / (var28 / 2.0);
					if (var41 * var41 + var43 * var43 + var45 * var45 < 1.0)
					{
						if (level.getTile(bx, by, bz) == Tile::sand.id)
						{
							level.setTileNoUpdate(bx, by, bz, Tile::clay.id);
						}
					}
				}
			}
		}
	}

	return true;
}
