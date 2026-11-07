#include "world/level/levelgen/TheEndLevelSource.h"

#include "java/Number.h"
#include "util/Memory.h"
#include "util/Mth.h"
#include "world/level/Level.h"
#include "world/level/biome/Biome.h"
#include "world/level/biome/BiomeSource.h"
#include "world/level/chunk/LevelChunk.h"
#include "world/level/tile/SandTile.h"
#include "world/level/tile/Tile.h"

TheEndLevelSource::TheEndLevelSource(Level &level, long_t seed) :
	random(seed),
	lperlinNoise1(random, 16),
	lperlinNoise2(random, 16),
	perlinNoise1(random, 8),
	scaleNoise(random, 10),
	depthNoise(random, 16),
	level(level)
{
}

void TheEndLevelSource::prepareHeights(int_t x, int_t z, ubyte_t *tiles)
{
	const int_t cells = 2;
	const int_t xs = cells + 1;
	const int_t ys = 128 / 4 + 1;
	const int_t zs = cells + 1;
	getHeights(heights.data(), x * cells, 0, z * cells, xs, ys, zs);

	for (int_t cx = 0; cx < cells; cx++)
	{
		for (int_t cz = 0; cz < cells; cz++)
		{
			for (int_t cy = 0; cy < 128 / 4; cy++)
			{
				double yStep = 0.25;
				double s0 = heights[((cx + 0) * zs + cz + 0) * ys + cy + 0];
				double s1 = heights[((cx + 0) * zs + cz + 1) * ys + cy + 0];
				double s2 = heights[((cx + 1) * zs + cz + 0) * ys + cy + 0];
				double s3 = heights[((cx + 1) * zs + cz + 1) * ys + cy + 0];
				double d0 = (heights[((cx + 0) * zs + cz + 0) * ys + cy + 1] - s0) * yStep;
				double d1 = (heights[((cx + 0) * zs + cz + 1) * ys + cy + 1] - s1) * yStep;
				double d2 = (heights[((cx + 1) * zs + cz + 0) * ys + cy + 1] - s2) * yStep;
				double d3 = (heights[((cx + 1) * zs + cz + 1) * ys + cy + 1] - s3) * yStep;
				for (int_t yy = 0; yy < 4; yy++)
				{
					double xStep = 0.125;
					double a = s0;
					double b = s1;
					double da = (s2 - s0) * xStep;
					double db = (s3 - s1) * xStep;
					for (int_t xx = 0; xx < 8; xx++)
					{
						int_t index = ((xx + cx * 8) << 11) | ((cz * 8) << 7) | (cy * 4 + yy);
						int_t stride = 1 << 7;
						double zStep = 0.125;
						double v = a;
						double dv = (b - a) * zStep;
						for (int_t zz = 0; zz < 8; zz++)
						{
							tiles[index] = static_cast<ubyte_t>(v > 0.0 ? Tile::endStone.id : 0);
							index += stride;
							v += dv;
						}
						a += da;
						b += db;
					}
					s0 += d0;
					s1 += d1;
					s2 += d2;
					s3 += d3;
				}
			}
		}
	}
	// 1.0 buildSurfaces only rewrites stone, and this island has none
}

std::shared_ptr<LevelChunk> TheEndLevelSource::getChunk(int_t x, int_t z)
{
	random.setSeed(Java::longFromBits(static_cast<ulong_t>(x) * 341873128712ULL + static_cast<ulong_t>(z) * 132897987541ULL));
	std::shared_ptr<LevelChunk> chunk = Util::make_shared<LevelChunk>(level, x, z);
	prepareHeights(x, z, chunk->blocks.data());
	chunk->recalcHeightmap();
	return chunk;
}

void TheEndLevelSource::getHeights(double *out, int_t x, int_t y, int_t z, int_t xd, int_t yd, int_t zd)
{
	double xScale = 684.412;
	double yScale = 684.412;
	scaleNoise.getRegion(sr.data(), x, z, xd, zd, 1.121, 1.121, 0.5);
	depthNoise.getRegion(dr.data(), x, z, xd, zd, 200.0, 200.0, 0.5);
	xScale *= 2.0;
	perlinNoise1.getRegion(pnr.data(), x, y, z, xd, yd, zd, xScale / 80.0, yScale / 160.0, xScale / 80.0);
	lperlinNoise1.getRegion(ar.data(), x, y, z, xd, yd, zd, xScale, yScale, xScale);
	lperlinNoise2.getRegion(br.data(), x, y, z, xd, yd, zd, xScale, yScale, xScale);

	int_t p = 0;
	int_t pp = 0;
	for (int_t xx = 0; xx < xd; xx++)
	{
		for (int_t zz = 0; zz < zd; zz++)
		{
			double scale = (sr[pp] + 256.0) / 512.0;
			if (scale > 1.0)
				scale = 1.0;
			double depth = dr[pp] / 8000.0;
			if (depth < 0.0)
				depth = -depth * 0.3;
			depth = depth * 3.0 - 2.0;
			float fx = static_cast<float>(xx + x);
			float fz = static_cast<float>(zz + z);
			float island = 100.0f - Mth::sqrt(fx * fx + fz * fz) * 8.0f;
			if (island > 80.0f)
				island = 80.0f;
			if (island < -100.0f)
				island = -100.0f;
			if (depth > 1.0)
				depth = 1.0;
			depth /= 8.0;
			depth = 0.0;
			if (scale < 0.0)
				scale = 0.0;
			scale += 0.5;
			depth = depth * yd / 16.0;
			pp++;
			double center = yd / 2.0;
			for (int_t yy = 0; yy < yd; yy++)
			{
				double val = 0.0;
				double off = (yy - center) * 8.0 / scale;
				if (off < 0.0)
					off *= -1.0;
				double lo = ar[p] / 512.0;
				double hi = br[p] / 512.0;
				double mix = (pnr[p] / 10.0 + 1.0) / 2.0;
				if (mix < 0.0)
					val = lo;
				else if (mix > 1.0)
					val = hi;
				else
					val = lo + (hi - lo) * mix;
				val -= 8.0;
				val += island;
				int_t slide = 2;
				if (yy > yd / 2 - slide)
				{
					double t = static_cast<float>(yy - (yd / 2 - slide)) / 64.0f;
					if (t < 0.0)
						t = 0.0;
					if (t > 1.0)
						t = 1.0;
					val = val * (1.0 - t) + -3000.0 * t;
				}
				slide = 8;
				if (yy < slide)
				{
					double t = static_cast<float>(slide - yy) / (static_cast<float>(slide) - 1.0f);
					val = val * (1.0 - t) + -30.0 * t;
				}
				out[p] = val;
				p++;
			}
		}
	}
}

bool TheEndLevelSource::hasChunk(int_t x, int_t z)
{
	(void)x;
	(void)z;
	return true;
}

void TheEndLevelSource::postProcess(ChunkSource &parent, int_t x, int_t z)
{
	(void)parent;
	SandTile::fallInstantly = true;
	int_t xo = x * 16;
	int_t zo = z * 16;
	Biome *biome = level.getBiomeSource().getBiome(xo + 16, zo + 16);
	biome->decorate(level, level.random, xo, zo);
	SandTile::fallInstantly = false;
}

bool TheEndLevelSource::save(bool force, std::shared_ptr<ProgressListener> progressListener)
{
	(void)force;
	(void)progressListener;
	return true;
}

bool TheEndLevelSource::tick()
{
	return false;
}

bool TheEndLevelSource::shouldSave()
{
	return true;
}

jstring TheEndLevelSource::gatherStats()
{
	return u"RandomLevelSource";
}
