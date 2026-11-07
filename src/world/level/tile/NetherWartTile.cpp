#include "world/level/tile/SoulSandTile.h"
#include "world/level/tile/NetherWartTile.h"

#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/level/Level.h"
#include "world/level/biome/Biome.h"
#include "world/level/biome/BiomeSource.h"

NetherWartTile::NetherWartTile(int_t id) : FlowerTile(id, 226)
{
	setTicking(true);
	updateDefaultShape();
}

void NetherWartTile::updateDefaultShape()
{
	float r = 0.5f;
	setShape(0.5f - r, 0.0f, 0.5f - r, 0.5f + r, 0.25f, 0.5f + r);
}

bool NetherWartTile::canSurviveOn(int_t belowTile) const
{
	return belowTile == Tile::soulSand.id;
}

void NetherWartTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	int_t data = level.getData(x, y, z);
	if (data < 3)
	{
		Biome *biome = level.getBiomeSource().getBiome(x, z);
		if (biome == Biome::hell && random.nextInt(15) == 0)
			level.setData(x, y, z, ++data);
	}
	FlowerTile::tick(level, x, y, z, random);
}

int_t NetherWartTile::getTexture(Facing face, int_t data)
{
	(void)face;
	if (data >= 3)
		return tex + 2;
	return data > 0 ? tex + 1 : tex;
}

void NetherWartTile::spawnResources(Level &level, int_t x, int_t y, int_t z, int_t data, float chance)
{
	(void)chance;
	if (level.isOnline)
		return;
	int_t count = 1;
	if (data >= 3)
		count = 2 + level.random.nextInt(3);
	for (int_t i = 0; i < count; i++)
		popResource(level, x, y, z, ItemInstance(Items::netherWart->getShiftedIndex(), 1, 0));
}

int_t NetherWartTile::getResource(int_t data, Random &random)
{
	(void)data;
	(void)random;
	return 0;
}

int_t NetherWartTile::getResourceCount(Random &random)
{
	(void)random;
	return 0;
}
