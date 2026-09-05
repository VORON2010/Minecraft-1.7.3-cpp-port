#include "world/level/dimension/HellDimension.h"

#include "world/level/Level.h"
#include "world/level/biome/BiomeSource.h"
#include "world/level/tile/Tile.h"
#include "world/level/levelgen/HellRandomLevelSource.h"
#include "world/level/chunk/storage/McRegionChunkStorage.h"

HellDimension::HellDimension(Level &level) : Dimension(level)
{
	biomeSource = std::make_unique<HellBiomeSource>(level);
	foggy = true;
	ultraWarm = true;
	hasCeiling = true;
	id = -1;
	updateLightRamp();
}

void HellDimension::updateLightRamp()
{
	float f = 0.1f;
	for (int_t i = 0; i < 16; i++)
	{
		float f1 = 1.0f - i / 15.0f;
		brightnessRamp[i] = (1.0f - f1) / (f1 * 3.0f + 1.0f) * (1.0f - f) + f;
	}
}

ChunkSource *HellDimension::createRandomLevelSource()
{
	return new HellRandomLevelSource(level, level.seed);
}

ChunkStorage *HellDimension::createStorage(std::shared_ptr<File> dir)
{
	std::shared_ptr<File> dimDir(File::open(*dir, u"DIM-1"));
	dimDir->mkdirs();
	return new McRegionChunkStorage(dimDir, true);
}

bool HellDimension::isValidSpawn(int_t x, int_t z)
{
	int_t top = level.getTopTile(x, z);
	if (top == Tile::bedrock.id)
		return false;
	if (top == 0)
		return false;
	return Tile::solid[top];
}

float HellDimension::getTimeOfDay(long_t time, float add)
{
	(void)time; (void)add;
	return 0.5f;
}

Vec3 *HellDimension::getFogColor(float time, float unknown)
{
	(void)time; (void)unknown;
	return Vec3::newTemp(0.2f, 0.03f, 0.03f);
}

bool HellDimension::mayRespawn()
{
	return false;
}
