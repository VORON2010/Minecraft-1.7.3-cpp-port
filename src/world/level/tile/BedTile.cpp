#include "world/level/tile/BedTile.h"

#include "world/entity/player/Player.h"
#include "world/item/Item.h"
#include "world/item/Items.h"
#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/WoodTile.h"
#include "world/level/Explosion.h"

constexpr int_t BedTile::headBlockToFootBlockMap[4][2];

BedTile::BedTile(int_t id, int_t tex) : Tile(id, tex, Material::cloth)
{
	updateDefaultShape();
	updateCachedProperties();
}

bool BedTile::use(Level &level, int_t x, int_t y, int_t z, Player &player)
{
	if (level.isOnline)
		return true;

	int_t data = level.getData(x, y, z);
	if (!isBlockFootOfBed(data))
	{
		int_t dir = getDirectionFromMetadata(data);
		x += headBlockToFootBlockMap[dir][0];
		z += headBlockToFootBlockMap[dir][1];
		if (level.getTile(x, y, z) != id)
			return true;
		data = level.getData(x, y, z);
	}

	if (!level.dimension->mayRespawn())
	{
		level.setTile(x, y, z, 0);
		int_t dir = getDirectionFromMetadata(data);
		int_t hx = x + headBlockToFootBlockMap[dir][0];
		int_t hz = z + headBlockToFootBlockMap[dir][1];
		if (level.getTile(hx, y, hz) == id)
			level.setTile(hx, y, hz, 0);
		
		
		(void)level.createExplosion(nullptr, hx + 0.5f, y + 0.5f, hz + 0.5f, 5.0f, true);
		return true;
	}

	if (isBedOccupied(data))
	{
		Player *occupant = nullptr;
		for (const auto &p : level.players)
		{
			if (p->isPlayerSleeping() && p->bedX == x && p->bedY == y && p->bedZ == z)
			{
				occupant = p.get();
				break;
			}
		}

		if (occupant != nullptr)
		{
			player.displayClientMessage(u"tile.bed.occupied");
			return true;
		}

		setBedOccupied(level, x, y, z, false);
	}

	Player::SleepStatus status = player.sleepInBedAt(x, y, z);
	if (status == Player::SleepStatus::OK)
	{
		setBedOccupied(level, x, y, z, true);
		return true;
	}
	else if (status == Player::SleepStatus::NOT_POSSIBLE_NOW)
	{
		player.displayClientMessage(u"tile.bed.noSleep");
		return true;
	}

	return true;
}

int_t BedTile::getTexture(Facing face, int_t data)
{
	if (face == Facing::DOWN)
		return Tile::wood.tex;

	int_t dir = getDirectionFromMetadata(data);

	static constexpr int bedDirection[4][6] = {
		{1, 0, 3, 2, 5, 4},
		{1, 0, 5, 4, 2, 3},
		{1, 0, 2, 3, 4, 5},
		{1, 0, 4, 5, 3, 2}
	};

	int_t mappedFace = bedDirection[dir][static_cast<int_t>(face)];

	if (isBlockFootOfBed(data))
	{
		if (mappedFace == 2)
			return tex + 2 + 16;
		else if (mappedFace != 5 && mappedFace != 4)
			return tex + 1;
		else
			return tex + 1 + 16;
	}
	else
	{
		if (mappedFace == 3)
			return tex - 1 + 16;
		else if (mappedFace != 5 && mappedFace != 4)
			return tex;
		else
			return tex + 16;
	}
}

void BedTile::neighborChanged(Level &level, int_t x, int_t y, int_t z, int_t tile)
{
	(void)tile;
	int_t data = level.getData(x, y, z);
	int_t dir = getDirectionFromMetadata(data);
	if (isBlockFootOfBed(data))
	{
		int_t headX = x - headBlockToFootBlockMap[dir][0];
		int_t headZ = z - headBlockToFootBlockMap[dir][1];
		if (level.getTile(headX, y, headZ) != id)
			level.setTile(x, y, z, 0);
	}
	else
	{
		int_t footX = x + headBlockToFootBlockMap[dir][0];
		int_t footZ = z + headBlockToFootBlockMap[dir][1];
		if (level.getTile(footX, y, footZ) != id)
		{
			level.setTile(x, y, z, 0);
			if (!level.isOnline)
				spawnResources(level, x, y, z, data);
		}
	}
}

int_t BedTile::getResource(int_t data, Random &random)
{
	(void)random;
	return isBlockFootOfBed(data) ? 0 : Items::bed->getShiftedIndex();
}

void BedTile::spawnResources(Level &level, int_t x, int_t y, int_t z, int_t data, float chance)
{
	if (!isBlockFootOfBed(data))
		Tile::spawnResources(level, x, y, z, data, chance);
}

void BedTile::updateDefaultShape()
{
	setShape(0.0f, 0.0f, 0.0f, 1.0f, 9.0f / 16.0f, 1.0f);
}

void BedTile::setBedOccupied(Level &level, int_t x, int_t y, int_t z, bool occupied)
{
	int_t data = level.getData(x, y, z);
	if (occupied)
		data |= 4;
	else
		data &= ~4;
	level.setData(x, y, z, data);
}

bool BedTile::getNearestEmptySpot(Level &level, int_t x, int_t y, int_t z, int_t skip, int_t &outX, int_t &outY, int_t &outZ)
{
	int_t dir = getDirectionFromMetadata(level.getData(x, y, z));
	for (int_t step = 0; step <= 1; step++)
	{
		int_t x0 = x - headBlockToFootBlockMap[dir][0] * step - 1;
		int_t z0 = z - headBlockToFootBlockMap[dir][1] * step - 1;
		int_t x1 = x0 + 2;
		int_t z1 = z0 + 2;
		for (int_t cx = x0; cx <= x1; cx++)
		{
			for (int_t cz = z0; cz <= z1; cz++)
			{
				if (level.isBlockNormalCube(cx, y - 1, cz) && level.isEmptyTile(cx, y, cz) && level.isEmptyTile(cx, y + 1, cz))
				{
					if (skip <= 0)
					{
						outX = cx;
						outY = y;
						outZ = cz;
						return true;
					}
					skip--;
				}
			}
		}
	}
	return false;
}


