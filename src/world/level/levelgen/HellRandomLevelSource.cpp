#include "world/level/levelgen/HellRandomLevelSource.h"
#include "java/Number.h"
#include "util/Profiler.h"

#include "world/level/Level.h"
#include "world/level/chunk/LevelChunk.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/SandTile.h"
#include "world/level/tile/LiquidTile.h"
#include "world/level/tile/GravelTile.h"
#include "world/level/tile/SoulSandTile.h"
#include "world/level/tile/MushroomTile.h"

#include "world/level/levelgen/feature/FlowerFeature.h"
#include "world/level/levelgen/feature/HellFeatures.h"

#include "util/Memory.h"
#include "util/Mth.h"

HellRandomLevelSource::HellRandomLevelSource(Level &level, long_t seed) :
	random(seed),
	lperlinNoise1(random, 16),
	lperlinNoise2(random, 16),
	perlinNoise1(random, 8),
	perlinNoise2(random, 4),
	perlinNoise3(random, 4),
	scaleNoise(random, 10),
	depthNoise(random, 16),
	level(level)
{
}


void HellRandomLevelSource::prepareHeights(int_t x, int_t z, ubyte_t *tiles)
{
	int_t cw = 4;
	int_t seaLevel = 32;
	int_t cw1 = cw + 1;
	int_t ch1 = 17;

	getHeights(buffer.data(), x * cw, 0, z * cw, cw1, ch1, cw1);

	for (int_t xi = 0; xi < cw; xi++)
	{
		for (int_t zi = 0; zi < cw; zi++)
		{
			for (int_t yi = 0; yi < 16; yi++)
			{
				double ddiv = 0.125;
				double v000 = buffer[((xi + 0) * cw1 + zi + 0) * ch1 + yi + 0];
				double v010 = buffer[((xi + 0) * cw1 + zi + 1) * ch1 + yi + 0];
				double v100 = buffer[((xi + 1) * cw1 + zi + 0) * ch1 + yi + 0];
				double v110 = buffer[((xi + 1) * cw1 + zi + 1) * ch1 + yi + 0];
				double s000 = (buffer[((xi + 0) * cw1 + zi + 0) * ch1 + yi + 1] - v000) * ddiv;
				double s010 = (buffer[((xi + 0) * cw1 + zi + 1) * ch1 + yi + 1] - v010) * ddiv;
				double s100 = (buffer[((xi + 1) * cw1 + zi + 0) * ch1 + yi + 1] - v100) * ddiv;
				double s110 = (buffer[((xi + 1) * cw1 + zi + 1) * ch1 + yi + 1] - v110) * ddiv;

				for (int_t ys = 0; ys < 8; ys++)
				{
					double d1div = 0.25;
					double d1 = v000;
					double d2 = v010;
					double s1 = (v100 - v000) * d1div;
					double s2 = (v110 - v010) * d1div;

					for (int_t xs = 0; xs < 4; xs++)
					{
						int_t index = ((xs + xi * 4) << 11) | ((0 + zi * 4) << 7) | (yi * 8 + ys);
						int_t zStride = 128;
						double d2div = 0.25;
						double dv = d1;
						double sv = (d2 - d1) * d2div;

						for (int_t zs = 0; zs < 4; zs++)
						{
							int_t tile = 0;
							if (yi * 8 + ys < seaLevel)
								tile = Tile::calmLava.id;
							if (dv > 0.0)
								tile = Tile::netherrack.id;

							tiles[index] = static_cast<ubyte_t>(tile);
							index += zStride;
							dv += sv;
						}

						d1 += s1;
						d2 += s2;
					}

					v000 += s000;
					v010 += s010;
					v100 += s100;
					v110 += s110;
				}
			}
		}
	}
}


void HellRandomLevelSource::buildSurfaces(int_t x, int_t z, ubyte_t *tiles)
{
	int_t seaLevel = 64;
	double scale = 1.0 / 32.0;
	perlinNoise2.getRegion(sandBuffer.data(), x * 16, z * 16, 0.0, 16, 16, 1, scale, scale, 1.0);
	perlinNoise2.getRegion(gravelBuffer.data(), x * 16, 109.0134, z * 16, 16, 1, 16, scale, 1.0, scale);
	perlinNoise3.getRegion(depthBuffer.data(), x * 16, z * 16, 0.0, 16, 16, 1, scale * 2.0, scale * 2.0, scale * 2.0);

	for (int_t xi = 0; xi < 16; xi++)
	{
		for (int_t zi = 0; zi < 16; zi++)
		{
			bool sand = sandBuffer[xi + zi * 16] + random.nextDouble() * 0.2 > 0.0;
			bool gravel = gravelBuffer[xi + zi * 16] + random.nextDouble() * 0.2 > 0.0;
			int_t depth = static_cast<int_t>(depthBuffer[xi + zi * 16] / 3.0 + 3.0 + random.nextDouble() * 0.25);
			int_t depthLeft = -1;
			ubyte_t topTile = static_cast<ubyte_t>(Tile::netherrack.id);
			ubyte_t fillTile = static_cast<ubyte_t>(Tile::netherrack.id);

			for (int_t y = 127; y >= 0; y--)
			{
				int_t index = (zi * 16 + xi) * 128 + y;
				if (y >= 127 - random.nextInt(5))
				{
					tiles[index] = static_cast<ubyte_t>(Tile::bedrock.id);
				}
				else if (y <= 0 + random.nextInt(5))
				{
					tiles[index] = static_cast<ubyte_t>(Tile::bedrock.id);
				}
				else
				{
					ubyte_t tile = tiles[index];
					if (tile == 0)
					{
						depthLeft = -1;
					}
					else if (tile == Tile::netherrack.id)
					{
						if (depthLeft == -1)
						{
							if (depth <= 0)
							{
								topTile = 0;
								fillTile = static_cast<ubyte_t>(Tile::netherrack.id);
							}
							else if (y >= seaLevel - 4 && y <= seaLevel + 1)
							{
								topTile = static_cast<ubyte_t>(Tile::netherrack.id);
								fillTile = static_cast<ubyte_t>(Tile::netherrack.id);
								if (gravel)
									topTile = static_cast<ubyte_t>(Tile::gravel.id);
								if (gravel)
									fillTile = static_cast<ubyte_t>(Tile::netherrack.id);
								if (sand)
									topTile = static_cast<ubyte_t>(Tile::soulSand.id);
								if (sand)
									fillTile = static_cast<ubyte_t>(Tile::soulSand.id);
							}

							if (y < seaLevel && topTile == 0)
								topTile = static_cast<ubyte_t>(Tile::calmLava.id);

							depthLeft = depth;
							if (y >= seaLevel - 1)
								tiles[index] = topTile;
							else
								tiles[index] = fillTile;
						}
						else if (depthLeft > 0)
						{
							depthLeft--;
							tiles[index] = fillTile;
						}
					}
				}
			}
		}
	}
}

std::shared_ptr<LevelChunk> HellRandomLevelSource::getChunk(int_t x, int_t z)
{
	Profiler::Scope profile(Profiler::Section::ChunkGeneration);
	random.setSeed(Java::longFromBits(static_cast<ulong_t>(x) * 341873128712ULL + static_cast<ulong_t>(z) * 132897987541ULL));

	std::shared_ptr<LevelChunk> chunk = Util::make_shared<LevelChunk>(level, x, z);

	prepareHeights(x, z, chunk->blocks.data());
	buildSurfaces(x, z, chunk->blocks.data());

	caveFeature.apply(*this, level, x, z, chunk->blocks);

	return chunk;
}


void HellRandomLevelSource::getHeights(double *out, int_t x, int_t y, int_t z, int_t xd, int_t yd, int_t zd)
{
	double xscale = 684.412;
	double yscale = 2053.236;

	scaleNoise.getRegion(sr.data(), x, y, z, xd, 1, zd, 1.0, 0.0, 1.0);
	depthNoise.getRegion(dr.data(), x, y, z, xd, 1, zd, 100.0, 0.0, 100.0);
	perlinNoise1.getRegion(pnr.data(), x, y, z, xd, yd, zd, xscale / 80.0, yscale / 60.0, xscale / 80.0);
	lperlinNoise1.getRegion(ar.data(), x, y, z, xd, yd, zd, xscale, yscale, xscale);
	lperlinNoise2.getRegion(br.data(), x, y, z, xd, yd, zd, xscale, yscale, xscale);

	int_t bufferIndex = 0;
	int_t flatIndex = 0;
	std::vector<double> yFalloff(static_cast<std::size_t>(yd));

	for (int_t yi = 0; yi < yd; yi++)
	{
		yFalloff[yi] = std::cos(yi * 3.1415926535897931 * 6.0 / yd) * 2.0;
		double dist = yi;
		if (yi > yd / 2)
			dist = yd - 1 - yi;

		if (dist < 4.0)
		{
			dist = 4.0 - dist;
			yFalloff[yi] -= dist * dist * dist * 10.0;
		}
	}

	for (int_t xi = 0; xi < xd; xi++)
	{
		for (int_t zi = 0; zi < zd; zi++)
		{
			double s = (sr[flatIndex] + 256.0) / 512.0;
			if (s > 1.0)
				s = 1.0;

			double lowerCutoff = 0.0;
			double d = dr[flatIndex] / 8000.0;
			if (d < 0.0)
				d = -d;

			d = d * 3.0 - 3.0;
			if (d < 0.0)
			{
				d /= 2.0;
				if (d < -1.0)
					d = -1.0;

				d /= 1.4;
				d /= 2.0;
				s = 0.0;
			}
			else
			{
				if (d > 1.0)
					d = 1.0;

				d /= 6.0;
			}

			s += 0.5;
			d = d * yd / 16.0;
			flatIndex++;

			for (int_t yi = 0; yi < yd; yi++)
			{
				double value = 0.0;
				double falloff = yFalloff[yi];
				double a = ar[bufferIndex] / 512.0;
				double b = br[bufferIndex] / 512.0;
				double p = (pnr[bufferIndex] / 10.0 + 1.0) / 2.0;
				if (p < 0.0)
					value = a;
				else if (p > 1.0)
					value = b;
				else
					value = a + (b - a) * p;

				value -= falloff;
				if (yi > yd - 4)
				{
					double t = static_cast<float>(yi - (yd - 4)) / 3.0f;
					value = value * (1.0 - t) + -10.0 * t;
				}

				if (yi < lowerCutoff)
				{
					double t = (lowerCutoff - yi) / 4.0;
					if (t < 0.0)
						t = 0.0;
					if (t > 1.0)
						t = 1.0;

					value = value * (1.0 - t) + -10.0 * t;
				}

				out[bufferIndex] = value;
				bufferIndex++;
			}
		}
	}
}

bool HellRandomLevelSource::hasChunk(int_t x, int_t z)
{
	(void)x; (void)z;
	return true;
}


void HellRandomLevelSource::postProcess(ChunkSource &parent, int_t x, int_t z)
{
	(void)parent;
	SandTile::fallInstantly = true;
	int_t cx = x * 16;
	int_t cz = z * 16;

	for (int_t i = 0; i < 8; i++)
	{
		int_t px = cx + random.nextInt(16) + 8;
		int_t py = random.nextInt(120) + 4;
		int_t pz = cz + random.nextInt(16) + 8;
		HellLavaFeature(Tile::lava.id).place(level, random, px, py, pz);
	}

	int_t fires = random.nextInt(random.nextInt(10) + 1) + 1;
	for (int_t i = 0; i < fires; i++)
	{
		int_t px = cx + random.nextInt(16) + 8;
		int_t py = random.nextInt(120) + 4;
		int_t pz = cz + random.nextInt(16) + 8;
		FireFeature().place(level, random, px, py, pz);
	}

	int_t glowstone = random.nextInt(random.nextInt(10) + 1);
	for (int_t i = 0; i < glowstone; i++)
	{
		int_t px = cx + random.nextInt(16) + 8;
		int_t py = random.nextInt(120) + 4;
		int_t pz = cz + random.nextInt(16) + 8;
		GlowStone1Feature().place(level, random, px, py, pz);
	}

	for (int_t i = 0; i < 10; i++)
	{
		int_t px = cx + random.nextInt(16) + 8;
		int_t py = random.nextInt(128);
		int_t pz = cz + random.nextInt(16) + 8;
		GlowStone2Feature().place(level, random, px, py, pz);
	}

	if (random.nextInt(1) == 0)
	{
		int_t px = cx + random.nextInt(16) + 8;
		int_t py = random.nextInt(128);
		int_t pz = cz + random.nextInt(16) + 8;
		FlowerFeature(Tile::brownMushroom.id).place(level, random, px, py, pz);
	}

	if (random.nextInt(1) == 0)
	{
		int_t px = cx + random.nextInt(16) + 8;
		int_t py = random.nextInt(128);
		int_t pz = cz + random.nextInt(16) + 8;
		FlowerFeature(Tile::redMushroom.id).place(level, random, px, py, pz);
	}

	SandTile::fallInstantly = false;
}

bool HellRandomLevelSource::save(bool force, std::shared_ptr<ProgressListener> progressListener)
{
	(void)force; (void)progressListener;
	return true;
}

bool HellRandomLevelSource::tick()
{
	return false;
}

bool HellRandomLevelSource::shouldSave()
{
	return true;
}

jstring HellRandomLevelSource::gatherStats()
{
	return u"HellRandomLevelSource";
}
