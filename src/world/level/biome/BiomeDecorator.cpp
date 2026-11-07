#include "world/level/biome/BiomeDecorator.h"

#include <stdexcept>

#include "world/level/Level.h"
#include "world/level/biome/Biome.h"
#include "world/level/levelgen/feature/BlockPatchFeature.h"
#include "world/level/levelgen/feature/CactusFeature.h"
#include "world/level/levelgen/feature/ClayFeature.h"
#include "world/level/levelgen/feature/DeadBushFeature.h"
#include "world/level/levelgen/feature/FlowerFeature.h"
#include "world/level/levelgen/feature/HugeMushroomFeature.h"
#include "world/level/levelgen/feature/OreFeature.h"
#include "world/level/levelgen/feature/PumpkinFeature.h"
#include "world/level/levelgen/feature/ReedFeature.h"
#include "world/level/levelgen/feature/SpringFeature.h"
#include "world/level/levelgen/feature/TallGrassFeature.h"
#include "world/level/levelgen/feature/WaterlilyFeature.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/DeadBushTile.h"
#include "world/level/tile/FlowerTile.h"
#include "world/level/tile/GravelTile.h"
#include "world/level/tile/LiquidTile.h"
#include "world/level/tile/MushroomTile.h"
#include "world/level/tile/OreTile.h"
#include "world/level/tile/RedstoneOreTile.h"
#include "world/level/tile/SandTile.h"
#include "world/level/tile/TallGrassTile.h"
#include "world/level/tile/DirtTile.h"

BiomeDecorator::BiomeDecorator(Biome &biome) : biome(biome)
{
	clayFeature = std::make_unique<ClayFeature>(4);
	sandFeature = std::make_unique<BlockPatchFeature>(7, Tile::sand.id);
	gravelFeature = std::make_unique<BlockPatchFeature>(6, Tile::gravel.id);
	dirtOreFeature = std::make_unique<OreFeature>(Tile::dirt.id, 32);
	gravelOreFeature = std::make_unique<OreFeature>(Tile::gravel.id, 32);
	coalOreFeature = std::make_unique<OreFeature>(Tile::coalOre.id, 16);
	ironOreFeature = std::make_unique<OreFeature>(Tile::ironOre.id, 8);
	goldOreFeature = std::make_unique<OreFeature>(Tile::goldOre.id, 8);
	redStoneOreFeature = std::make_unique<OreFeature>(Tile::redstoneOre.id, 7);
	diamondOreFeature = std::make_unique<OreFeature>(Tile::diamondOre.id, 7);
	lapisOreFeature = std::make_unique<OreFeature>(Tile::lapisOre.id, 6);
	yellowFlowerFeature = std::make_unique<FlowerFeature>(Tile::flower.id);
	roseFlowerFeature = std::make_unique<FlowerFeature>(Tile::rose.id);
	brownMushroomFeature = std::make_unique<FlowerFeature>(Tile::brownMushroom.id);
	redMushroomFeature = std::make_unique<FlowerFeature>(Tile::redMushroom.id);
	hugeMushroomFeature = std::make_unique<HugeMushroomFeature>();
	reedsFeature = std::make_unique<ReedFeature>();
	cactusFeature = std::make_unique<CactusFeature>();
	waterlilyFeature = std::make_unique<WaterlilyFeature>();
}

BiomeDecorator::~BiomeDecorator() = default;

void BiomeDecorator::decorate(Level &level, Random &random, int_t xo, int_t zo)
{
	if (this->level != nullptr)
		throw std::runtime_error("Already decorating!!");
	this->level = &level;
	this->random = &random;
	this->xo = xo;
	this->zo = zo;
	decorate();
	this->level = nullptr;
	this->random = nullptr;
}

void BiomeDecorator::decorate()
{
	Level &level = *this->level;
	Random &random = *this->random;
	const int_t depth = Level::DEPTH;

	decorateOres();

	for (int_t i = 0; i < sandCount; ++i)
	{
		int_t x = xo + random.nextInt(16) + 8;
		int_t z = zo + random.nextInt(16) + 8;
		sandFeature->place(level, random, x, level.getTopSolidBlock(x, z), z);
	}

	for (int_t i = 0; i < clayCount; ++i)
	{
		int_t x = xo + random.nextInt(16) + 8;
		int_t z = zo + random.nextInt(16) + 8;
		clayFeature->place(level, random, x, level.getTopSolidBlock(x, z), z);
	}

	// vanilla 1.0 places sand here too (gravelFeature is never used)
	for (int_t i = 0; i < gravelCount; ++i)
	{
		int_t x = xo + random.nextInt(16) + 8;
		int_t z = zo + random.nextInt(16) + 8;
		sandFeature->place(level, random, x, level.getTopSolidBlock(x, z), z);
	}

	int_t forests = treeCount;
	if (random.nextInt(10) == 0)
		forests++;

	for (int_t i = 0; i < forests; ++i)
	{
		int_t x = xo + random.nextInt(16) + 8;
		int_t z = zo + random.nextInt(16) + 8;
		Feature &tree = biome.getTreeFeature(random);
		tree.init(1.0, 1.0, 1.0);
		tree.place(level, random, x, level.getHeightmap(x, z), z);
	}

	for (int_t i = 0; i < hugeMushrooms; ++i)
	{
		int_t x = xo + random.nextInt(16) + 8;
		int_t z = zo + random.nextInt(16) + 8;
		hugeMushroomFeature->place(level, random, x, level.getHeightmap(x, z), z);
	}

	for (int_t i = 0; i < flowerCount; ++i)
	{
		int_t x = xo + random.nextInt(16) + 8;
		int_t y = random.nextInt(depth);
		int_t z = zo + random.nextInt(16) + 8;
		yellowFlowerFeature->place(level, random, x, y, z);
		if (random.nextInt(4) == 0)
		{
			x = xo + random.nextInt(16) + 8;
			y = random.nextInt(depth);
			z = zo + random.nextInt(16) + 8;
			roseFlowerFeature->place(level, random, x, y, z);
		}
	}

	for (int_t i = 0; i < grassCount; ++i)
	{
		int_t data = 1;
		int_t x = xo + random.nextInt(16) + 8;
		int_t y = random.nextInt(depth);
		int_t z = zo + random.nextInt(16) + 8;
		TallGrassFeature(Tile::tallGrass.id, data).place(level, random, x, y, z);
	}

	for (int_t i = 0; i < deadBushCount; ++i)
	{
		int_t x = xo + random.nextInt(16) + 8;
		int_t y = random.nextInt(depth);
		int_t z = zo + random.nextInt(16) + 8;
		DeadBushFeature(Tile::deadBush.id).place(level, random, x, y, z);
	}

	for (int_t i = 0; i < waterlilyCount; ++i)
	{
		int_t x = xo + random.nextInt(16) + 8;
		int_t z = zo + random.nextInt(16) + 8;
		int_t y = random.nextInt(depth);
		while (y > 0 && level.getTile(x, y - 1, z) == 0)
			y--;
		waterlilyFeature->place(level, random, x, y, z);
	}

	for (int_t i = 0; i < mushroomCount; ++i)
	{
		if (random.nextInt(4) == 0)
		{
			int_t x = xo + random.nextInt(16) + 8;
			int_t z = zo + random.nextInt(16) + 8;
			int_t y = level.getHeightmap(x, z);
			brownMushroomFeature->place(level, random, x, y, z);
		}
		if (random.nextInt(8) == 0)
		{
			int_t x = xo + random.nextInt(16) + 8;
			int_t z = zo + random.nextInt(16) + 8;
			int_t y = random.nextInt(depth);
			redMushroomFeature->place(level, random, x, y, z);
		}
	}

	if (random.nextInt(4) == 0)
	{
		int_t x = xo + random.nextInt(16) + 8;
		int_t y = random.nextInt(depth);
		int_t z = zo + random.nextInt(16) + 8;
		brownMushroomFeature->place(level, random, x, y, z);
	}

	if (random.nextInt(8) == 0)
	{
		int_t x = xo + random.nextInt(16) + 8;
		int_t y = random.nextInt(depth);
		int_t z = zo + random.nextInt(16) + 8;
		redMushroomFeature->place(level, random, x, y, z);
	}

	for (int_t i = 0; i < reedsCount; ++i)
	{
		int_t x = xo + random.nextInt(16) + 8;
		int_t z = zo + random.nextInt(16) + 8;
		int_t y = random.nextInt(depth);
		reedsFeature->place(level, random, x, y, z);
	}

	for (int_t i = 0; i < 10; ++i)
	{
		int_t x = xo + random.nextInt(16) + 8;
		int_t y = random.nextInt(depth);
		int_t z = zo + random.nextInt(16) + 8;
		reedsFeature->place(level, random, x, y, z);
	}

	if (random.nextInt(32) == 0)
	{
		int_t x = xo + random.nextInt(16) + 8;
		int_t y = random.nextInt(depth);
		int_t z = zo + random.nextInt(16) + 8;
		PumpkinFeature().place(level, random, x, y, z);
	}

	for (int_t i = 0; i < cactusCount; ++i)
	{
		int_t x = xo + random.nextInt(16) + 8;
		int_t y = random.nextInt(depth);
		int_t z = zo + random.nextInt(16) + 8;
		cactusFeature->place(level, random, x, y, z);
	}

	if (liquids)
	{
		for (int_t i = 0; i < 50; ++i)
		{
			int_t x = xo + random.nextInt(16) + 8;
			int_t y = random.nextInt(random.nextInt(depth - 8) + 8);
			int_t z = zo + random.nextInt(16) + 8;
			SpringFeature(Tile::water.id).place(level, random, x, y, z);
		}

		for (int_t i = 0; i < 20; ++i)
		{
			int_t x = xo + random.nextInt(16) + 8;
			int_t y = random.nextInt(random.nextInt(random.nextInt(depth - 16) + 8) + 8);
			int_t z = zo + random.nextInt(16) + 8;
			SpringFeature(Tile::lava.id).place(level, random, x, y, z);
		}
	}
}

void BiomeDecorator::decorateDepthSpan(int_t count, Feature &feature, int_t y0, int_t y1)
{
	for (int_t i = 0; i < count; ++i)
	{
		int_t x = xo + random->nextInt(16);
		int_t y = random->nextInt(y1 - y0) + y0;
		int_t z = zo + random->nextInt(16);
		feature.place(*level, *random, x, y, z);
	}
}

void BiomeDecorator::decorateDepthAverage(int_t count, Feature &feature, int_t yMid, int_t ySpan)
{
	for (int_t i = 0; i < count; ++i)
	{
		int_t x = xo + random->nextInt(16);
		int_t y = random->nextInt(ySpan) + random->nextInt(ySpan) + (yMid - ySpan);
		int_t z = zo + random->nextInt(16);
		feature.place(*level, *random, x, y, z);
	}
}

void BiomeDecorator::decorateOres()
{
	const int_t depth = Level::DEPTH;
	decorateDepthSpan(20, *dirtOreFeature, 0, depth);
	decorateDepthSpan(10, *gravelOreFeature, 0, depth);
	decorateDepthSpan(20, *coalOreFeature, 0, depth);
	decorateDepthSpan(20, *ironOreFeature, 0, depth / 2);
	decorateDepthSpan(2, *goldOreFeature, 0, depth / 4);
	decorateDepthSpan(8, *redStoneOreFeature, 0, depth / 8);
	decorateDepthSpan(1, *diamondOreFeature, 0, depth / 8);
	decorateDepthAverage(1, *lapisOreFeature, depth / 8, depth / 8);
}
