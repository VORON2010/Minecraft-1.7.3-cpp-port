#include "world/level/dimension/TheEndDimension.h"

#include "util/Mth.h"
#include "world/level/Level.h"
#include "world/level/biome/Biome.h"
#include "world/level/biome/BiomeSource.h"
#include "world/level/chunk/storage/McRegionChunkStorage.h"
#include "world/level/levelgen/TheEndLevelSource.h"
#include "world/level/material/Material.h"
#include "world/level/tile/Tile.h"

TheEndDimension::TheEndDimension(Level &level) : Dimension(level)
{
	Biome::init();
	biomeSource = std::make_unique<FixedBiomeSource>(Biome::sky, 0.5f, 0.0f);
	id = 1;
	hasCeiling = true;
	foggy = true;
	updateLightRamp();
}

ChunkSource *TheEndDimension::createRandomLevelSource()
{
	return new TheEndLevelSource(level, level.seed);
}

ChunkStorage *TheEndDimension::createStorage(std::shared_ptr<File> dir)
{
	std::shared_ptr<File> dimDir(File::open(*dir, u"DIM1"));
	dimDir->mkdirs();
	return new McRegionChunkStorage(dimDir, true);
}

float TheEndDimension::getTimeOfDay(long_t time, float add)
{
	(void)time;
	(void)add;
	return 0.0f;
}

float *TheEndDimension::getSunriseColor(float time, float unknown)
{
	(void)time;
	(void)unknown;
	return nullptr;
}

Vec3 *TheEndDimension::getFogColor(float time, float unknown)
{
	(void)unknown;
	int_t col = 8421536;
	float br = Mth::cos(time * Mth::PI * 2.0f) * 2.0f + 0.5f;
	if (br < 0.0f)
		br = 0.0f;
	if (br > 1.0f)
		br = 1.0f;
	float r = static_cast<float>(col >> 16 & 0xFF) / 255.0f;
	float g = static_cast<float>(col >> 8 & 0xFF) / 255.0f;
	float b = static_cast<float>(col & 0xFF) / 255.0f;
	r *= br * 0.0f + 0.15f;
	g *= br * 0.0f + 0.15f;
	b *= br * 0.0f + 0.15f;
	return Vec3::newTemp(r, g, b);
}

bool TheEndDimension::hasGround()
{
	return false;
}

bool TheEndDimension::mayRespawn()
{
	return false;
}

float TheEndDimension::getCloudHeight()
{
	return 8.0f;
}

bool TheEndDimension::isValidSpawn(int_t x, int_t z)
{
	int_t top = level.getTopTile(x, z);
	if (top == 0)
		return false;
	return Tile::tiles[top]->material.blocksMotion();
}

bool TheEndDimension::getForcedSpawnPoint(int_t &x, int_t &y, int_t &z)
{
	x = 100;
	y = 50;
	z = 0;
	return true;
}
