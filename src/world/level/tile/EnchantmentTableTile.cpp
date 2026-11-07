#include "world/level/tile/EnchantmentTableTile.h"

#include <cmath>

#include "client/player/LocalPlayer.h"
#include "util/Memory.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/tile/BookshelfTile.h"

Random EnchantmentTableTileEntity::random;

EnchantmentTableTile::EnchantmentTableTile(int_t id) : Tile(id, 166, Material::stone)
{
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.75f, 1.0f);
	Tile::isEntityTile[id] = true;
	Tile::lightBlock[id] = 0;
}

int_t EnchantmentTableTile::getTexture(Facing face, int_t data)
{
	(void)data;
	return getTexture(face);
}

int_t EnchantmentTableTile::getTexture(Facing face)
{
	if (face == Facing::DOWN)
		return tex + 17;
	return face == Facing::UP ? tex : tex + 16;
}

void EnchantmentTableTile::onPlace(Level &level, int_t x, int_t y, int_t z)
{
	Tile::onPlace(level, x, y, z);
	level.setTileEntity(x, y, z, Util::make_shared<EnchantmentTableTileEntity>());
}

void EnchantmentTableTile::onRemove(Level &level, int_t x, int_t y, int_t z)
{
	Tile::onRemove(level, x, y, z);
	level.removeTileEntity(x, y, z);
}

void EnchantmentTableTile::animateTick(Level &level, int_t x, int_t y, int_t z, Random &random)
{
	Tile::animateTick(level, x, y, z, random);
	for (int_t xx = x - 2; xx <= x + 2; ++xx)
	{
		for (int_t zz = z - 2; zz <= z + 2; ++zz)
		{
			if (xx > x - 2 && xx < x + 2 && zz == z - 1)
				zz = z + 2;
			if (random.nextInt(16) != 0)
				continue;
			for (int_t yy = y; yy <= y + 1; ++yy)
			{
				if (level.getTile(xx, yy, zz) != Tile::bookshelf.id)
					continue;
				if (!level.isEmptyTile((xx - x) / 2 + x, yy, (zz - z) / 2 + z))
					break;
				level.addParticle(u"enchantmenttable", x + 0.5, y + 2.0, z + 0.5,
					(xx - x) + random.nextFloat() - 0.5, (yy - y) - random.nextFloat() - 1.0f, (zz - z) + random.nextFloat() - 0.5);
			}
		}
	}
}

bool EnchantmentTableTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	if (level.isOnline)
		return true;
	if (LocalPlayer *local = dynamic_cast<LocalPlayer *>(&player))
		local->startEnchanting(x, y, z);
	return true;
}

void EnchantmentTableTileEntity::tick()
{
	TileEntity::tick();
	oOpen = open;
	oRot = rot;
	std::shared_ptr<Player> player = level != nullptr ? level->getNearestPlayer(x + 0.5f, y + 0.5f, z + 0.5f, 3.0) : nullptr;
	const float pi = 3.1415927f;
	if (player != nullptr)
	{
		double dx = player->x - (x + 0.5f);
		double dz = player->z - (z + 0.5f);
		tRot = static_cast<float>(std::atan2(dz, dx));
		open += 0.1f;
		if (open < 0.5f || random.nextInt(40) == 0)
		{
			float old = flipT;
			do
				flipT += static_cast<float>(random.nextInt(4) - random.nextInt(4));
			while (old == flipT);
		}
	}
	else
	{
		tRot += 0.02f;
		open -= 0.1f;
	}
	while (rot >= pi)
		rot -= pi * 2.0f;
	while (rot < -pi)
		rot += pi * 2.0f;
	while (tRot >= pi)
		tRot -= pi * 2.0f;
	while (tRot < -pi)
		tRot += pi * 2.0f;
	float d = tRot - rot;
	while (d >= pi)
		d -= pi * 2.0f;
	while (d < -pi)
		d += pi * 2.0f;
	rot += d * 0.4f;
	if (open < 0.0f)
		open = 0.0f;
	if (open > 1.0f)
		open = 1.0f;
	time++;
	oFlip = flip;
	float step = (flipT - flip) * 0.4f;
	float max = 0.2f;
	if (step < -max)
		step = -max;
	if (step > max)
		step = max;
	flipA += (step - flipA) * 0.9f;
	flip += flipA;
}
