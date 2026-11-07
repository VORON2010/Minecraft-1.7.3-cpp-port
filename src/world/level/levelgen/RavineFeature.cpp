// Ported from the 1.0 ravine carver.
#include "world/level/levelgen/RavineFeature.h"

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

void RavineFeature::addRavine(long_t seed, int_t chunkX, int_t chunkZ, Blocks &blocks, double x, double y, double z, float baseWidth, float yaw, float pitch, int_t tunnel, int_t tunnelCount, double widthHeightRatio)
{

		Random var19(seed);
		double var20 = (double)(chunkX * 16 + 8);
		double var22 = (double)(chunkZ * 16 + 8);
		float var24 = 0.0f;
		float var25 = 0.0f;
		if (tunnelCount <= 0) {
			int_t var26 = radius * 16 - 16;
			tunnelCount = var26 - var19.nextInt(var26 / 4);
		}

		bool var56 = false;
		if (tunnel == -1) {
			tunnel = tunnelCount / 2;
			var56 = true;
		}

		float var27 = 1.0f;

		for (int_t var28 = 0; var28 < Level::DEPTH; var28++) {
			if (var28 == 0 || var19.nextInt(3) == 0) {
				var27 = 1.0f + var19.nextFloat() * var19.nextFloat() * 1.0f;
			}

			buffer[var28] = var27 * var27;
		}

		for (; tunnel < tunnelCount; tunnel++) {
			double var57 = 1.5 + (double)(Mth::sin((float)tunnel * Mth::PI / (float)tunnelCount) * baseWidth * 1.0f);
			double var30 = var57 * widthHeightRatio;
			var57 *= (double)var19.nextFloat() * 0.25 + 0.75;
			var30 *= (double)var19.nextFloat() * 0.25 + 0.75;
			float var32 = Mth::cos(pitch);
			float var33 = Mth::sin(pitch);
			x += (double)(Mth::cos(yaw) * var32);
			y += (double)var33;
			z += (double)(Mth::sin(yaw) * var32);
			pitch *= 0.7f;
			pitch += var25 * 0.05f;
			yaw += var24 * 0.05f;
			var25 *= 0.8f;
			var24 *= 0.5f;
			var25 += (var19.nextFloat() - var19.nextFloat()) * var19.nextFloat() * 2.0f;
			var24 += (var19.nextFloat() - var19.nextFloat()) * var19.nextFloat() * 4.0f;
			if (var56 || var19.nextInt(4) != 0) {
				double var34 = x - var20;
				double var36 = z - var22;
				double var38 = (double)(tunnelCount - tunnel);
				double var40 = (double)(baseWidth + 2.0f + 16.0f);
				if (var34 * var34 + var36 * var36 - var38 * var38 > var40 * var40) {
					return;
				}

				if (!(x < var20 - 16.0 - var57 * 2.0)
					&& !(z < var22 - 16.0 - var57 * 2.0)
					&& !(x > var20 + 16.0 + var57 * 2.0)
					&& !(z > var22 + 16.0 + var57 * 2.0)) {
					int_t var60 = Mth::floor(x - var57) - chunkX * 16 - 1;
					int_t var35 = Mth::floor(x + var57) - chunkX * 16 + 1;
					int_t var61 = Mth::floor(y - var30) - 1;
					int_t var37 = Mth::floor(y + var30) + 1;
					int_t var62 = Mth::floor(z - var57) - chunkZ * 16 - 1;
					int_t var39 = Mth::floor(z + var57) - chunkZ * 16 + 1;
					if (var60 < 0) {
						var60 = 0;
					}

					if (var35 > 16) {
						var35 = 16;
					}

					if (var61 < 1) {
						var61 = 1;
					}

					if (var37 > Level::DEPTH - 8) {
						var37 = Level::DEPTH - 8;
					}

					if (var62 < 0) {
						var62 = 0;
					}

					if (var39 > 16) {
						var39 = 16;
					}

					bool var63 = false;

					for (int_t var41 = var60; !var63 && var41 < var35; var41++) {
						for (int_t var42 = var62; !var63 && var42 < var39; var42++) {
							for (int_t var43 = var37 + 1; !var63 && var43 >= var61 - 1; var43--) {
								int_t var44 = (var41 * 16 + var42) * Level::DEPTH + var43;
								if (var43 >= 0 && var43 < Level::DEPTH) {
									if (blocks[var44] == Tile::water.id || blocks[var44] == Tile::calmWater.id) {
										var63 = true;
									}

									if (var43 != var61 - 1 && var41 != var60 && var41 != var35 - 1 && var42 != var62 && var42 != var39 - 1) {
										var43 = var61;
									}
								}
							}
						}
					}

					if (!var63) {
						for (int_t var64 = var60; var64 < var35; var64++) {
							double var65 = ((double)(var64 + chunkX * 16) + 0.5 - x) / var57;

							for (int_t var66 = var62; var66 < var39; var66++) {
								double var45 = ((double)(var66 + chunkZ * 16) + 0.5 - z) / var57;
								int_t var47 = (var64 * 16 + var66) * Level::DEPTH + var37;
								bool var48 = false;
								if (var65 * var65 + var45 * var45 < 1.0) {
									for (int_t var49 = var37 - 1; var49 >= var61; var49--) {
										double var50 = ((double)var49 + 0.5 - y) / var30;
										if ((var65 * var65 + var45 * var45) * (double)buffer[var49] + var50 * var50 / 6.0 < 1.0) {
											byte_t var52 = blocks[var47];
											if (var52 == Tile::grass.id) {
												var48 = true;
											}

											if (var52 == Tile::rock.id || var52 == Tile::dirt.id || var52 == Tile::grass.id) {
												if (var49 < 10) {
													blocks[var47] = (byte_t)Tile::lava.id;
												} else {
													blocks[var47] = 0;
													if (var48 && blocks[var47 - 1] == Tile::dirt.id) {
														blocks[var47 - 1] = level->getBiomeSource().getBiome(var64 + chunkX * 16, var66 + chunkZ * 16)->topMaterial;
													}
												}
											}
										}

										var47--;
									}
								}
							}
						}

						if (var56) {
							break;
						}
					}
				}
			}
		}
	}

void RavineFeature::addFeature(Level &level, int_t startChunkX, int_t startChunkZ, int_t chunkX, int_t chunkZ, Blocks &blocks)
{
	(void)level;

		if (random.nextInt(50) == 0) {
			double var7 = (double)(startChunkX * 16 + random.nextInt(16));
			double var9 = (double)(random.nextInt(random.nextInt(40) + 8) + 20);
			double var11 = (double)(startChunkZ * 16 + random.nextInt(16));
			byte_t var13 = 1;

			for (int_t var14 = 0; var14 < var13; var14++) {
				float var15 = random.nextFloat() * Mth::PI * 2.0f;
				float var16 = (random.nextFloat() - 0.5f) * 2.0f / 8.0f;
				float var17 = (random.nextFloat() * 2.0f + random.nextFloat()) * 2.0f;
				addRavine(random.nextLong(), chunkX, chunkZ, blocks, var7, var9, var11, var17, var15, var16, 0, 0, 3.0);
			}
		}
	}
