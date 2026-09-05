#include "world/level/levelgen/feature/DungeonFeature.h"

#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"
#include "world/level/Level.h"
#include "world/level/material/Material.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/ChestTile.h"
#include "world/level/tile/MobSpawnerTile.h"
#include "world/level/tile/entity/ChestTileEntity.h"
#include "world/level/tile/entity/MobSpawnerTileEntity.h"

bool DungeonFeature::place(Level &level, Random &random, int_t x, int_t y, int_t z)
{
	int_t height = 3;
	int_t xr = random.nextInt(2) + 2;
	int_t zr = random.nextInt(2) + 2;
	int_t openings = 0;

	for (int_t xx = x - xr - 1; xx <= x + xr + 1; xx++)
	{
		for (int_t yy = y - 1; yy <= y + height + 1; yy++)
		{
			for (int_t zz = z - zr - 1; zz <= z + zr + 1; zz++)
			{
				const Material &material = level.getMaterial(xx, yy, zz);
				if (yy == y - 1 && !material.isSolid())
					return false;
				if (yy == y + height + 1 && !material.isSolid())
					return false;
				if ((xx == x - xr - 1 || xx == x + xr + 1 || zz == z - zr - 1 || zz == z + zr + 1)
					&& yy == y && level.isEmptyTile(xx, yy, zz) && level.isEmptyTile(xx, yy + 1, zz))
					openings++;
			}
		}
	}

	if (openings < 1 || openings > 5)
		return false;

	for (int_t xx = x - xr - 1; xx <= x + xr + 1; xx++)
	{
		for (int_t yy = y + height; yy >= y - 1; yy--)
		{
			for (int_t zz = z - zr - 1; zz <= z + zr + 1; zz++)
			{
				if (xx != x - xr - 1 && yy != y - 1 && zz != z - zr - 1 && xx != x + xr + 1 && yy != y + height + 1 && zz != z + zr + 1)
				{
					level.setTile(xx, yy, zz, 0);
				}
				else if (yy >= 0 && !level.getMaterial(xx, yy - 1, zz).isSolid())
				{
					level.setTile(xx, yy, zz, 0);
				}
				else if (level.getMaterial(xx, yy, zz).isSolid())
				{
					if (yy == y - 1 && random.nextInt(4) != 0)
						level.setTile(xx, yy, zz, Tile::mossyCobblestone.id);
					else
						level.setTile(xx, yy, zz, Tile::cobblestone.id);
				}
			}
		}
	}

	for (int_t attempt = 0; attempt < 2; attempt++)
	{
		for (int_t tries = 0; tries < 3; tries++)
		{
			int_t chestX = x + random.nextInt(xr * 2 + 1) - xr;
			int_t chestZ = z + random.nextInt(zr * 2 + 1) - zr;
			if (!level.isEmptyTile(chestX, y, chestZ))
				continue;

			int_t solidSides = 0;
			if (level.getMaterial(chestX - 1, y, chestZ).isSolid())
				solidSides++;
			if (level.getMaterial(chestX + 1, y, chestZ).isSolid())
				solidSides++;
			if (level.getMaterial(chestX, y, chestZ - 1).isSolid())
				solidSides++;
			if (level.getMaterial(chestX, y, chestZ + 1).isSolid())
				solidSides++;

			if (solidSides == 1)
			{
				level.setTile(chestX, y, chestZ, Tile::chest.id);
				auto te = level.getTileEntity(chestX, y, chestZ);
				auto *chest = dynamic_cast<ChestTileEntity *>(te.get());
				for (int_t i = 0; i < 8; i++)
				{
					ItemInstance loot = pickLootItem(random);
					if (loot.isEmpty())
						continue;
					int_t slot = random.nextInt(chest != nullptr ? chest->getContainerSize() : 27);
					if (chest != nullptr)
						chest->setItem(slot, loot);
				}
				break;
			}
		}
	}

	level.setTile(x, y, z, Tile::mobSpawner.id);
	jstring mob = pickMob(random);
	auto te = level.getTileEntity(x, y, z);
	auto *spawner = dynamic_cast<MobSpawnerTileEntity *>(te.get());
	if (spawner != nullptr)
		spawner->setEntityId(mob);
	return true;
}

ItemInstance DungeonFeature::pickLootItem(Random &random)
{
	int_t r = random.nextInt(11);
	if (r == 0)
		return ItemInstance(Items::saddle->getShiftedIndex());
	if (r == 1)
		return ItemInstance(Items::ingotIron->getShiftedIndex(), random.nextInt(4) + 1);
	if (r == 2)
		return ItemInstance(Items::bread->getShiftedIndex());
	if (r == 3)
		return ItemInstance(Items::wheat->getShiftedIndex(), random.nextInt(4) + 1);
	if (r == 4)
		return ItemInstance(Items::gunpowder->getShiftedIndex(), random.nextInt(4) + 1);
	if (r == 5)
		return ItemInstance(Items::silk->getShiftedIndex(), random.nextInt(4) + 1);
	if (r == 6)
		return ItemInstance(Items::bucketEmpty->getShiftedIndex());
	if (r == 7 && random.nextInt(100) == 0)
		return ItemInstance(Items::appleGold->getShiftedIndex());
	if (r == 8 && random.nextInt(2) == 0)
		return ItemInstance(Items::redstone->getShiftedIndex(), random.nextInt(4) + 1);
	if (r == 9 && random.nextInt(10) == 0)
		return ItemInstance(Items::record13->getShiftedIndex() + random.nextInt(2));
	if (r == 10)
		return ItemInstance(Items::dyePowder->getShiftedIndex(), 1, 3);
	return ItemInstance();
}

jstring DungeonFeature::pickMob(Random &random)
{
	int_t r = random.nextInt(4);
	if (r == 0)
		return u"Skeleton";
	if (r == 1)
		return u"Zombie";
	if (r == 2)
		return u"Zombie";
	if (r == 3)
		return u"Spider";
	return u"";
}
