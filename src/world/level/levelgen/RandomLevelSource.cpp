// Overworld generator from release 1.0, ported from OverworldChunkGenerator.
#include "world/level/levelgen/RandomLevelSource.h"
#include "java/Number.h"
#include "util/Mth.h"
#include "util/Profiler.h"

#include "world/level/Level.h"
#include "world/level/SpawnerAnimals.h"
#include "world/level/biome/Biome.h"
#include "world/level/biome/BiomeSource.h"
#include "world/level/chunk/LevelChunk.h"
#include "world/level/levelgen/feature/DungeonFeature.h"
#include "world/level/levelgen/feature/LakeFeature.h"
#include "world/level/levelgen/structure/MineShaftFeature.h"
#include "world/level/levelgen/structure/StrongholdFeature.h"
#include "world/level/levelgen/structure/VillageFeature.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/IceTile.h"
#include "world/level/tile/LiquidTile.h"
#include "world/level/tile/SandStoneTile.h"
#include "world/level/tile/SandTile.h"
#include "world/level/tile/SnowTile.h"
#include "world/level/tile/StoneTile.h"

RandomLevelSource::RandomLevelSource(Level &level, long_t seed, bool generateStructures) : random(seed),
	lperlinNoise1(random, 16),
	lperlinNoise2(random, 16),
	perlinNoise1(random, 8),
	perlinNoise3(random, 4),
	scaleNoise(random, 10),
	depthNoise(random, 16),
	forestNoise(random, 8),
	level(level),
	generateStructures(generateStructures)
{
	strongholdFeature = std::make_unique<StrongholdFeature>();
	villageFeature = std::make_unique<VillageFeature>();
	mineShaftFeature = std::make_unique<MineShaftFeature>();
}

RandomLevelSource::~RandomLevelSource() = default;

void RandomLevelSource::prepareHeights(int_t chunkX, int_t chunkZ, ubyte_t *blocks)
{
	const int_t xzCells = 4;
	const int_t yCells = Level::DEPTH / 8;
	const int_t xs = xzCells + 1;
	const int_t ys = Level::DEPTH / 8 + 1;
	const int_t zs = xzCells + 1;

	level.getBiomeSource().getRawBiomeBlock(biomes, chunkX * 4 - 2, chunkZ * 4 - 2, xs + 5, zs + 5);
	getHeights(buffer, chunkX * xzCells, 0, chunkZ * xzCells, xs, ys, zs);

	for (int_t xc = 0; xc < xzCells; xc++)
	{
		for (int_t zc = 0; zc < xzCells; zc++)
		{
			for (int_t yc = 0; yc < yCells; yc++)
			{
				double yStep = 0.125;
				double s0 = buffer[((xc + 0) * zs + zc + 0) * ys + yc + 0];
				double s1 = buffer[((xc + 0) * zs + zc + 1) * ys + yc + 0];
				double s2 = buffer[((xc + 1) * zs + zc + 0) * ys + yc + 0];
				double s3 = buffer[((xc + 1) * zs + zc + 1) * ys + yc + 0];
				double s0a = (buffer[((xc + 0) * zs + zc + 0) * ys + yc + 1] - s0) * yStep;
				double s1a = (buffer[((xc + 0) * zs + zc + 1) * ys + yc + 1] - s1) * yStep;
				double s2a = (buffer[((xc + 1) * zs + zc + 0) * ys + yc + 1] - s2) * yStep;
				double s3a = (buffer[((xc + 1) * zs + zc + 1) * ys + yc + 1] - s3) * yStep;

				for (int_t y = 0; y < 8; y++)
				{
					double xStep = 0.25;
					double z0 = s0;
					double z1 = s1;
					double z0a = (s2 - s0) * xStep;
					double z1a = (s3 - s1) * xStep;

					for (int_t x = 0; x < 4; x++)
					{
						int_t offs = (x + xc * 4) << 11 | (0 + zc * 4) << 7 | (yc * 8 + y);
						const int_t step = 1 << 7;
						offs -= step;
						double zStep = 0.25;
						double value = z0;
						double valueStep = (z1 - z0) * zStep;
						value -= valueStep;

						for (int_t z = 0; z < 4; z++)
						{
							if ((value += valueStep) > 0.0)
								blocks[offs += step] = static_cast<ubyte_t>(Tile::rock.id);
							else if (yc * 8 + y < SEA_LEVEL)
								blocks[offs += step] = static_cast<ubyte_t>(Tile::calmWater.id);
							else
								blocks[offs += step] = 0;
						}

						z0 += z0a;
						z1 += z1a;
					}

					s0 += s0a;
					s1 += s1a;
					s2 += s2a;
					s3 += s3a;
				}
			}
		}
	}
}

void RandomLevelSource::buildSurfaces(int_t chunkX, int_t chunkZ, ubyte_t *blocks, std::vector<Biome *> &biomes)
{
	double scale = 0.03125;
	perlinNoise3.getRegion(depthBuffer.data(), chunkX * 16, chunkZ * 16, 0, 16, 16, 1, scale * 2.0, scale * 2.0, scale * 2.0);
	std::array<float, 256> temperatures;
	level.getBiomeSource().getTemperatureBlock(temperatures.data(), chunkX * 16, chunkZ * 16, 16, 16);

	for (int_t x = 0; x < 16; x++)
	{
		for (int_t z = 0; z < 16; z++)
		{
			float temperature = temperatures[z + x * 16];
			Biome *biome = biomes[z + x * 16];
			int_t runDepth = static_cast<int_t>(depthBuffer[x + z * 16] / 3.0 + 3.0 + random.nextDouble() * 0.25);
			int_t run = -1;
			ubyte_t top = biome->topMaterial;
			ubyte_t material = biome->material;

			for (int_t y = Level::DEPTH - 1; y >= 0; y--)
			{
				int_t offs = (z * 16 + x) * Level::DEPTH + y;
				if (y <= 0 + random.nextInt(5))
				{
					blocks[offs] = static_cast<ubyte_t>(Tile::bedrock.id);
					continue;
				}

				ubyte_t old = blocks[offs];
				if (old == 0)
				{
					run = -1;
				}
				else if (old == Tile::rock.id)
				{
					if (run == -1)
					{
						if (runDepth <= 0)
						{
							top = 0;
							material = static_cast<ubyte_t>(Tile::rock.id);
						}
						else if (y >= SEA_LEVEL - 4 && y <= SEA_LEVEL + 1)
						{
							top = biome->topMaterial;
							material = biome->material;
						}

						if (y < SEA_LEVEL && top == 0)
						{
							if (temperature < 0.15f)
								top = static_cast<ubyte_t>(Tile::ice.id);
							else
								top = static_cast<ubyte_t>(Tile::calmWater.id);
						}

						run = runDepth;
						if (y >= SEA_LEVEL - 1)
							blocks[offs] = top;
						else
							blocks[offs] = material;
					}
					else if (run > 0)
					{
						run--;
						blocks[offs] = material;
						if (run == 0 && material == Tile::sand.id)
						{
							run = random.nextInt(4);
							material = static_cast<ubyte_t>(Tile::sandstone.id);
						}
					}
				}
			}
		}
	}
}

std::shared_ptr<LevelChunk> RandomLevelSource::getChunk(int_t x, int_t z)
{
	Profiler::Scope profile(Profiler::Section::ChunkGeneration);
	random.setSeed(Java::longFromBits(static_cast<ulong_t>(static_cast<long_t>(x)) * 341873128712ULL + static_cast<ulong_t>(static_cast<long_t>(z)) * 132897987541ULL));

	std::shared_ptr<LevelChunk> chunk = Util::make_shared<LevelChunk>(level, x, z);
	ubyte_t *blocks = chunk->blocks.data();

	prepareHeights(x, z, blocks);
	level.getBiomeSource().getBiomeBlock(biomes, x * 16, z * 16, 16, 16);
	buildSurfaces(x, z, blocks, biomes);

	caveFeature.apply(*this, level, x, z, chunk->blocks);
	canyonFeature.apply(*this, level, x, z, chunk->blocks);
	if (generateStructures)
	{
		mineShaftFeature->apply(*this, level, x, z, chunk->blocks);
		villageFeature->apply(*this, level, x, z, chunk->blocks);
		strongholdFeature->apply(*this, level, x, z, chunk->blocks);
	}

	chunk->recalcHeightmap();
	return chunk;
}

void RandomLevelSource::getHeights(std::vector<double> &out, int_t x, int_t y, int_t z, int_t xSize, int_t ySize, int_t zSize)
{
	out.assign(static_cast<size_t>(xSize * ySize * zSize), 0.0);

	if (!biomeWeightsReady)
	{
		for (int_t xb = -2; xb <= 2; xb++)
			for (int_t zb = -2; zb <= 2; zb++)
				biomeWeights[xb + 2 + (zb + 2) * 5] = 10.0f / Mth::sqrt(static_cast<float>(xb * xb + zb * zb) + 0.2f);
		biomeWeightsReady = true;
	}

	double s = 684.412;
	double hs = 684.412;
	sr.resize(static_cast<size_t>(xSize * zSize));
	dr.resize(static_cast<size_t>(xSize * zSize));
	pnr.resize(out.size());
	ar.resize(out.size());
	br.resize(out.size());
	scaleNoise.getRegion(sr.data(), x, z, xSize, zSize, 1.121, 1.121, 0.5);
	depthNoise.getRegion(dr.data(), x, z, xSize, zSize, 200.0, 200.0, 0.5);
	perlinNoise1.getRegion(pnr.data(), x, y, z, xSize, ySize, zSize, s / 80.0, hs / 160.0, s / 80.0);
	lperlinNoise1.getRegion(ar.data(), x, y, z, xSize, ySize, zSize, s, hs, s);
	lperlinNoise2.getRegion(br.data(), x, y, z, xSize, ySize, zSize, s, hs, s);

	int_t p = 0;
	int_t pp = 0;
	for (int_t xx = 0; xx < xSize; xx++)
	{
		for (int_t zz = 0; zz < zSize; zz++)
		{
			float sss = 0.0f;
			float ddd = 0.0f;
			float pow = 0.0f;
			const int_t rr = 2;
			Biome *center = biomes[xx + 2 + (zz + 2) * (xSize + 5)];

			for (int_t xb = -rr; xb <= rr; xb++)
			{
				for (int_t zb = -rr; zb <= rr; zb++)
				{
					Biome *b = biomes[xx + xb + 2 + (zz + zb + 2) * (xSize + 5)];
					float weight = biomeWeights[xb + 2 + (zb + 2) * 5] / (b->depth + 2.0f);
					if (b->depth > center->depth)
						weight /= 2.0f;
					sss += b->scale * weight;
					ddd += b->depth * weight;
					pow += weight;
				}
			}

			sss /= pow;
			ddd /= pow;
			sss = sss * 0.9f + 0.1f;
			ddd = (ddd * 4.0f - 1.0f) / 8.0f;

			double rdepth = dr[pp] / 8000.0;
			if (rdepth < 0.0)
				rdepth = -rdepth * 0.3;
			rdepth = rdepth * 3.0 - 2.0;
			if (rdepth < 0.0)
			{
				rdepth /= 2.0;
				if (rdepth < -1.0)
					rdepth = -1.0;
				rdepth /= 1.4;
				rdepth /= 2.0;
			}
			else
			{
				if (rdepth > 1.0)
					rdepth = 1.0;
				rdepth /= 8.0;
			}
			pp++;

			for (int_t yy = 0; yy < ySize; yy++)
			{
				double depth = ddd;
				double scale = sss;
				depth += rdepth * 0.2;
				depth = depth * ySize / 16.0;
				double yCenter = ySize / 2.0 + depth * 4.0;
				double val = 0.0;
				double yOffs = (yy - yCenter) * 12.0 * 128.0 / Level::DEPTH / scale;
				if (yOffs < 0.0)
					yOffs *= 4.0;

				double bb = ar[p] / 512.0;
				double cc = br[p] / 512.0;
				double v = (pnr[p] / 10.0 + 1.0) / 2.0;
				if (v < 0.0)
					val = bb;
				else if (v > 1.0)
					val = cc;
				else
					val = bb + (cc - bb) * v;
				val -= yOffs;

				if (yy > ySize - 4)
				{
					double slide = static_cast<double>(static_cast<float>(yy - (ySize - 4)) / 3.0f);
					val = val * (1.0 - slide) + -10.0 * slide;
				}

				out[p] = val;
				p++;
			}
		}
	}
}

bool RandomLevelSource::hasChunk(int_t x, int_t z)
{
	(void)x;
	(void)z;
	return true;
}

void RandomLevelSource::postProcess(ChunkSource &parent, int_t chunkX, int_t chunkZ)
{
	(void)parent;
	SandTile::fallInstantly = true;
	int_t xo = chunkX * 16;
	int_t zo = chunkZ * 16;
	Biome *biome = level.getBiomeSource().getBiome(xo + 16, zo + 16);

	random.setSeed(level.seed);
	long_t xScale = random.nextLong() / 2LL * 2LL + 1LL;
	long_t zScale = random.nextLong() / 2LL * 2LL + 1LL;
	random.setSeed(Java::longFromBits((static_cast<ulong_t>(static_cast<long_t>(chunkX)) * static_cast<ulong_t>(xScale) +
		static_cast<ulong_t>(static_cast<long_t>(chunkZ)) * static_cast<ulong_t>(zScale)) ^ static_cast<ulong_t>(level.seed)));

	bool hasVillage = false;
	if (generateStructures)
	{
		mineShaftFeature->postProcess(level, random, chunkX, chunkZ);
		hasVillage = villageFeature->postProcess(level, random, chunkX, chunkZ);
		strongholdFeature->postProcess(level, random, chunkX, chunkZ);
	}

	if (!hasVillage && random.nextInt(4) == 0)
	{
		int_t x = xo + random.nextInt(16) + 8;
		int_t y = random.nextInt(Level::DEPTH);
		int_t z = zo + random.nextInt(16) + 8;
		LakeFeature(Tile::calmWater.id).place(level, random, x, y, z);
	}

	if (!hasVillage && random.nextInt(8) == 0)
	{
		int_t x = xo + random.nextInt(16) + 8;
		int_t y = random.nextInt(random.nextInt(Level::DEPTH - 8) + 8);
		int_t z = zo + random.nextInt(16) + 8;
		if (y < SEA_LEVEL || random.nextInt(10) == 0)
			LakeFeature(Tile::calmLava.id).place(level, random, x, y, z);
	}

	for (int_t i = 0; i < 8; i++)
	{
		int_t x = xo + random.nextInt(16) + 8;
		int_t y = random.nextInt(Level::DEPTH);
		int_t z = zo + random.nextInt(16) + 8;
		DungeonFeature().place(level, random, x, y, z);
	}

	biome->decorate(level, random, xo, zo);
	SpawnerAnimals::postProcessSpawnMobs(level, *biome, xo + 8, zo + 8, 16, 16, random);
	xo += 8;
	zo += 8;

	for (int_t x = 0; x < 16; x++)
	{
		for (int_t z = 0; z < 16; z++)
		{
			int_t y = level.getTopRainBlock(xo + x, zo + z);
			if (level.shouldFreeze(x + xo, y - 1, z + zo))
				level.setTile(x + xo, y - 1, z + zo, Tile::ice.id);
			if (level.shouldSnow(x + xo, y, z + zo))
				level.setTile(x + xo, y, z + zo, Tile::snow.id);
		}
	}

	SandTile::fallInstantly = false;
}

bool RandomLevelSource::save(bool force, std::shared_ptr<ProgressListener> progressListener)
{
	(void)force;
	(void)progressListener;
	return true;
}

bool RandomLevelSource::tick()
{
	return false;
}

bool RandomLevelSource::shouldSave()
{
	return true;
}

jstring RandomLevelSource::gatherStats()
{
	return u"RandomLevelSource";
}

bool RandomLevelSource::findNearestStronghold(int_t x, int_t y, int_t z, int_t &outX, int_t &outY, int_t &outZ)
{
	if (strongholdFeature == nullptr)
		return false;
	return strongholdFeature->findNearest(level, x, y, z, outX, outY, outZ);
}
