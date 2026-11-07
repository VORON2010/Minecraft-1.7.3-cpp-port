// Ported from the 1.0 cave carver.
#include "world/level/levelgen/LargeCaveFeature.h"

#include <cmath>

#include "util/Mth.h"
#include "world/level/Level.h"
#include "world/level/biome/Biome.h"
#include "world/level/biome/BiomeSource.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/StoneTile.h"
#include "world/level/tile/GrassTile.h"
#include "world/level/tile/DirtTile.h"
#include "world/level/tile/LiquidTile.h"

void LargeCaveFeature::addRoom(long_t seed, int_t chunkX, int_t chunkZ, Blocks &blocks, double x, double y, double z)
{
	addTunnel(seed, chunkX, chunkZ, blocks, x, y, z, 1.0f + random.nextFloat() * 6.0f, 0.0f, 0.0f, -1, -1, 0.5);
}

void LargeCaveFeature::addTunnel(long_t seed, int_t chunkX, int_t chunkZ, Blocks &blocks, double x, double y, double z, float baseWidth, float yaw, float pitch, int_t tunnel, int_t tunnelCount, double widthHeightRatio)
{

		double var19 = (double)(chunkX * 16 + 8);
		double var21 = (double)(chunkZ * 16 + 8);
		float var23 = 0.0f;
		float var24 = 0.0f;
		Random var25(seed);
		if (tunnelCount <= 0) {
			int_t var26 = radius * 16 - 16;
			tunnelCount = var26 - var25.nextInt(var26 / 4);
		}

		bool var57 = false;
		if (tunnel == -1) {
			tunnel = tunnelCount / 2;
			var57 = true;
		}

		int_t var27 = var25.nextInt(tunnelCount / 2) + tunnelCount / 4;

		for (bool var28 = var25.nextInt(6) == 0; tunnel < tunnelCount; tunnel++) {
			double var29 = 1.5 + (double)(Mth::sin((float)tunnel * Mth::PI / (float)tunnelCount) * baseWidth * 1.0f);
			double var31 = var29 * widthHeightRatio;
			float var33 = Mth::cos(pitch);
			float var34 = Mth::sin(pitch);
			x += (double)(Mth::cos(yaw) * var33);
			y += (double)var34;
			z += (double)(Mth::sin(yaw) * var33);
			if (var28) {
				pitch *= 0.92f;
			} else {
				pitch *= 0.7f;
			}

			pitch += var24 * 0.1f;
			yaw += var23 * 0.1f;
			var24 *= 0.9f;
			var23 *= 0.75f;
			var24 += (var25.nextFloat() - var25.nextFloat()) * var25.nextFloat() * 2.0f;
			var23 += (var25.nextFloat() - var25.nextFloat()) * var25.nextFloat() * 4.0f;
			if (!var57 && tunnel == var27 && baseWidth > 1.0f && tunnelCount > 0) {
				addTunnel(
					var25.nextLong(),
					chunkX,
					chunkZ,
					blocks,
					x,
					y,
					z,
					var25.nextFloat() * 0.5f + 0.5f,
					yaw - (float) (3.141592653589793 / 2),
					pitch / 3.0f,
					tunnel,
					tunnelCount,
					1.0
				);
				addTunnel(
					var25.nextLong(),
					chunkX,
					chunkZ,
					blocks,
					x,
					y,
					z,
					var25.nextFloat() * 0.5f + 0.5f,
					yaw + (float) (3.141592653589793 / 2),
					pitch / 3.0f,
					tunnel,
					tunnelCount,
					1.0
				);
				return;
			}

			if (var57 || var25.nextInt(4) != 0) {
				double var35 = x - var19;
				double var37 = z - var21;
				double var39 = (double)(tunnelCount - tunnel);
				double var41 = (double)(baseWidth + 2.0f + 16.0f);
				if (var35 * var35 + var37 * var37 - var39 * var39 > var41 * var41) {
					return;
				}

				if (!(x < var19 - 16.0 - var29 * 2.0)
					&& !(z < var21 - 16.0 - var29 * 2.0)
					&& !(x > var19 + 16.0 + var29 * 2.0)
					&& !(z > var21 + 16.0 + var29 * 2.0)) {
					int_t var58 = Mth::floor(x - var29) - chunkX * 16 - 1;
					int_t var36 = Mth::floor(x + var29) - chunkX * 16 + 1;
					int_t var59 = Mth::floor(y - var31) - 1;
					int_t var38 = Mth::floor(y + var31) + 1;
					int_t var60 = Mth::floor(z - var29) - chunkZ * 16 - 1;
					int_t var40 = Mth::floor(z + var29) - chunkZ * 16 + 1;
					if (var58 < 0) {
						var58 = 0;
					}

					if (var36 > 16) {
						var36 = 16;
					}

					if (var59 < 1) {
						var59 = 1;
					}

					if (var38 > Level::DEPTH - 8) {
						var38 = Level::DEPTH - 8;
					}

					if (var60 < 0) {
						var60 = 0;
					}

					if (var40 > 16) {
						var40 = 16;
					}

					bool var61 = false;

					for (int_t var42 = var58; !var61 && var42 < var36; var42++) {
						for (int_t var43 = var60; !var61 && var43 < var40; var43++) {
							for (int_t var44 = var38 + 1; !var61 && var44 >= var59 - 1; var44--) {
								int_t var45 = (var42 * 16 + var43) * Level::DEPTH + var44;
								if (var44 >= 0 && var44 < Level::DEPTH) {
									if (blocks[var45] == Tile::water.id || blocks[var45] == Tile::calmWater.id) {
										var61 = true;
									}

									if (var44 != var59 - 1 && var42 != var58 && var42 != var36 - 1 && var43 != var60 && var43 != var40 - 1) {
										var44 = var59;
									}
								}
							}
						}
					}

					if (!var61) {
						for (int_t var62 = var58; var62 < var36; var62++) {
							double var63 = ((double)(var62 + chunkX * 16) + 0.5 - x) / var29;

							for (int_t var64 = var60; var64 < var40; var64++) {
								double var46 = ((double)(var64 + chunkZ * 16) + 0.5 - z) / var29;
								int_t var48 = (var62 * 16 + var64) * Level::DEPTH + var38;
								bool var49 = false;
								if (var63 * var63 + var46 * var46 < 1.0) {
									for (int_t var50 = var38 - 1; var50 >= var59; var50--) {
										double var51 = ((double)var50 + 0.5 - y) / var31;
										if (var51 > -0.7 && var63 * var63 + var51 * var51 + var46 * var46 < 1.0) {
											byte_t var53 = blocks[var48];
											if (var53 == Tile::grass.id) {
												var49 = true;
											}

											if (var53 == Tile::rock.id || var53 == Tile::dirt.id || var53 == Tile::grass.id) {
												if (var50 < 10) {
													blocks[var48] = (byte_t)Tile::lava.id;
												} else {
													blocks[var48] = 0;
													if (var49 && blocks[var48 - 1] == Tile::dirt.id) {
														blocks[var48 - 1] = level->getBiomeSource().getBiome(var62 + chunkX * 16, var64 + chunkZ * 16)->topMaterial;
													}
												}
											}
										}

										var48--;
									}
								}
							}
						}

						if (var57) {
							break;
						}
					}
				}
			}
		}
	}

void LargeCaveFeature::addFeature(Level &level, int_t startChunkX, int_t startChunkZ, int_t chunkX, int_t chunkZ, Blocks &blocks)
{
	(void)level;

		int_t var7 = random.nextInt(random.nextInt(random.nextInt(40) + 1) + 1);
		if (random.nextInt(15) != 0) {
			var7 = 0;
		}

		for (int_t var8 = 0; var8 < var7; var8++) {
			double var9 = (double)(startChunkX * 16 + random.nextInt(16));
			double var11 = (double)random.nextInt(random.nextInt(Level::DEPTH - 8) + 8);
			double var13 = (double)(startChunkZ * 16 + random.nextInt(16));
			int_t var15 = 1;
			if (random.nextInt(4) == 0) {
				addRoom(random.nextLong(), chunkX, chunkZ, blocks, var9, var11, var13);
				var15 += random.nextInt(4);
			}

			for (int_t var16 = 0; var16 < var15; var16++) {
				float var17 = random.nextFloat() * Mth::PI * 2.0f;
				float var18 = (random.nextFloat() - 0.5f) * 2.0f / 8.0f;
				float var19 = random.nextFloat() * 2.0f + random.nextFloat();
				if (random.nextInt(10) == 0) {
					var19 *= random.nextFloat() * random.nextFloat() * 3.0f + 1.0f;
				}

				addTunnel(random.nextLong(), chunkX, chunkZ, blocks, var9, var11, var13, var19, var17, var18, 0, 0, 1.0);
			}
		}
	}
