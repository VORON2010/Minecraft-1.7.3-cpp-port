#include "world/level/levelgen/feature/OreFeature.h"

#include "world/level/Level.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/StoneTile.h"

#include "util/Mth.h"

OreFeature::OreFeature(int_t blockId, int_t count) : blockId(blockId), count(count)
{
}

bool OreFeature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	float var6 = random.nextFloat() * Mth::PI;
	double var7 = (double)((float)(x + 8) + Mth::sin(var6) * (float)this->count / 8.0F);
	double var9 = (double)((float)(x + 8) - Mth::sin(var6) * (float)this->count / 8.0F);
	double var11 = (double)((float)(z + 8) + Mth::cos(var6) * (float)this->count / 8.0F);
	double var13 = (double)((float)(z + 8) - Mth::cos(var6) * (float)this->count / 8.0F);
	double var15 = (double)(y + random.nextInt(3) + 2);
	double var17 = (double)(y + random.nextInt(3) + 2);

	for (int_t var19 = 0; var19 <= this->count; ++var19)
	{
		double var20 = var7 + (var9 - var7) * (double)var19 / (double)this->count;
		double var22 = var15 + (var17 - var15) * (double)var19 / (double)this->count;
		double var24 = var11 + (var13 - var11) * (double)var19 / (double)this->count;
		double var26 = random.nextDouble() * (double)this->count / 16.0;
		double var28 = (double)(Mth::sin((float)var19 * Mth::PI / (float)this->count) + 1.0F) * var26 + 1.0;
		double var30 = (double)(Mth::sin((float)var19 * Mth::PI / (float)this->count) + 1.0F) * var26 + 1.0;
		int_t var32 = Mth::floor(var20 - var28 / 2.0);
		int_t var33 = Mth::floor(var22 - var30 / 2.0);
		int_t var34 = Mth::floor(var24 - var28 / 2.0);
		int_t var35 = Mth::floor(var20 + var28 / 2.0);
		int_t var36 = Mth::floor(var22 + var30 / 2.0);
		int_t var37 = Mth::floor(var24 + var28 / 2.0);

		for (int_t var38 = var32; var38 <= var35; ++var38)
		{
			double var39 = ((double)var38 + 0.5 - var20) / (var28 / 2.0);
			if (var39 * var39 < 1.0)
			{
				for (int_t var41 = var33; var41 <= var36; ++var41)
				{
					double var42 = ((double)var41 + 0.5 - var22) / (var30 / 2.0);
					if (var39 * var39 + var42 * var42 < 1.0)
					{
						for (int_t var44 = var34; var44 <= var37; ++var44)
						{
							double var45 = ((double)var44 + 0.5 - var24) / (var28 / 2.0);
							if (var39 * var39 + var42 * var42 + var45 * var45 < 1.0 && level.getTile(var38, var41, var44) == Tile::rock.id)
							{
								level.setTileNoUpdate(var38, var41, var44, this->blockId);
							}
						}
					}
				}
			}
		}
	}

	return true;
}
