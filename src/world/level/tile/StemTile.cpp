#include "world/level/tile/StemTile.h"

#include <memory>

#include "world/entity/item/EntityItem.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/level/Level.h"
#include "world/level/tile/FarmlandTile.h"
#include "world/level/tile/PumpkinTile.h"
#include "world/level/tile/MelonTile.h"

StemTile::StemTile(int_t id, Tile &fruit) : FlowerTile(id, 111), fruit(fruit)
{
	setTicking(true);
	updateDefaultShape();
}

int_t StemTile::getTexture(Facing face, int_t data)
{
	(void)face;
	(void)data;
	return tex;
}

bool StemTile::canSurviveOn(int_t belowTile) const
{
	return belowTile == Tile::farmland.id;
}

void StemTile::tick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	FlowerTile::tick(level, x, y, z, random);
	if (level.getTile(x, y, z) != id)
		return;
	if (level.getRawBrightness(x, y + 1, z) < 9)
		return;

	float speed = getGrowthSpeed(level, x, y, z);
	int_t delay = static_cast<int_t>(100.0f / speed);
	if (delay < 1)
		delay = 1;
	if (random.nextInt(delay) != 0)
		return;

	int_t data = level.getData(x, y, z);
	if (data < 7)
	{
		level.setData(x, y, z, data + 1);
		return;
	}

	if (level.getTile(x - 1, y, z) == fruit.id || level.getTile(x + 1, y, z) == fruit.id ||
		level.getTile(x, y, z - 1) == fruit.id || level.getTile(x, y, z + 1) == fruit.id)
		return;

	int_t dir = random.nextInt(4);
	int_t fx = x;
	int_t fz = z;
	if (dir == 0) fx--;
	if (dir == 1) fx++;
	if (dir == 2) fz--;
	if (dir == 3) fz++;

	if (level.getTile(fx, y, fz) == 0 && level.getTile(fx, y - 1, fz) == Tile::farmland.id)
		level.setTile(fx, y, fz, fruit.id);
}

void StemTile::growFully(Level &level, int_t x, int_t y, int_t z)
{
	level.setData(x, y, z, 7);
}

float StemTile::getGrowthSpeed(Level &level, int_t x, int_t y, int_t z)
{
	float speed = 1.0f;
	int_t north = level.getTile(x, y, z - 1);
	int_t south = level.getTile(x, y, z + 1);
	int_t west = level.getTile(x - 1, y, z);
	int_t east = level.getTile(x + 1, y, z);
	int_t nw = level.getTile(x - 1, y, z - 1);
	int_t ne = level.getTile(x + 1, y, z - 1);
	int_t se = level.getTile(x + 1, y, z + 1);
	int_t sw = level.getTile(x - 1, y, z + 1);
	bool adjX = west == id || east == id;
	bool adjZ = north == id || south == id;
	bool adjDiag = nw == id || ne == id || se == id || sw == id;

	for (int_t xx = x - 1; xx <= x + 1; ++xx)
	{
		for (int_t zz = z - 1; zz <= z + 1; ++zz)
		{
			float c = 0.0f;
			if (level.getTile(xx, y - 1, zz) == Tile::farmland.id)
			{
				c = 1.0f;
				if (level.getData(xx, y - 1, zz) > 0)
					c = 3.0f;
			}
			if (xx != x || zz != z)
				c /= 4.0f;
			speed += c;
		}
	}

	if (adjDiag || (adjX && adjZ))
		speed /= 2.0f;
	return speed;
}

int_t StemTile::getItemColor(int_t data)
{
	int_t r = data * 32;
	int_t g = 255 - data * 8;
	int_t b = data * 4;
	return r << 16 | g << 8 | b;
}

int_t StemTile::getColor(LevelSource &level, int_t x, int_t y, int_t z)
{
	return getItemColor(level.getData(x, y, z));
}

void StemTile::updateDefaultShape()
{
	float r = 0.125f;
	setShape(0.5f - r, 0.0f, 0.5f - r, 0.5f + r, 0.25f, 0.5f + r);
}

void StemTile::updateShape(LevelSource &level, int_t x, int_t y, int_t z)
{
	float height = static_cast<float>(level.getData(x, y, z) * 2 + 2) / 16.0f;
	float r = 0.125f;
	setShape(0.5f - r, 0.0f, 0.5f - r, 0.5f + r, height, 0.5f + r);
}

int_t StemTile::getConnectDirection(LevelSource &level, int_t x, int_t y, int_t z)
{
	if (level.getData(x, y, z) < 7)
		return -1;
	if (level.getTile(x - 1, y, z) == fruit.id) return 0;
	if (level.getTile(x + 1, y, z) == fruit.id) return 1;
	if (level.getTile(x, y, z - 1) == fruit.id) return 2;
	if (level.getTile(x, y, z + 1) == fruit.id) return 3;
	return -1;
}

int_t StemTile::getResource(int_t data, Random &random)
{
	(void)data;
	(void)random;
	return -1;
}

void StemTile::spawnResources(Level &level, int_t x, int_t y, int_t z, int_t data, float chance)
{
	Tile::spawnResources(level, x, y, z, data, chance);
	if (level.isOnline)
		return;

	Item *seed = &fruit == &Tile::pumpkin ? Items::pumpkinSeeds : Items::melonSeeds;
	for (int_t i = 0; i < 3; ++i)
	{
		if (level.random.nextInt(15) > data)
			continue;
		float spread = 0.7f;
		float xo = level.random.nextFloat() * spread + (1.0f - spread) * 0.5f;
		float yo = level.random.nextFloat() * spread + (1.0f - spread) * 0.5f;
		float zo = level.random.nextFloat() * spread + (1.0f - spread) * 0.5f;
		auto entity = std::make_shared<EntityItem>(level, x + xo, y + yo, z + zo, ItemInstance(seed->getShiftedIndex(), 1, 0));
		entity->throwTime = 10;
		level.addEntity(entity);
	}
}
