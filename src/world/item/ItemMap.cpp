#include "world/item/ItemMap.h"

#include "nbt/CompoundTag.h"
#include "util/Mth.h"
#include "world/entity/Entity.h"
#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/level/Level.h"
#include "world/level/MapData.h"
#include "world/level/chunk/LevelChunk.h"
#include "world/level/material/Material.h"
#include "world/level/material/MapColor.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/DirtTile.h"
#include "world/level/tile/StoneTile.h"

ItemMap::ItemMap(int_t baseId) : Item(baseId)
{
	setMaxStackSize(1);
}

MapData *ItemMap::getMapData(short_t mapId, Level &level)
{
	jstring id = u"map_" + String::toString(mapId);
	MapData *data = dynamic_cast<MapData *>(level.loadItemData(id, [](const jstring &s) { return std::make_unique<MapData>(s); }));
	if (data == nullptr)
	{
		int_t newId = level.getUniqueDataId(u"map");
		id = u"map_" + String::toString(newId);
		data = new MapData(id);
		level.setItemData(id, data);
	}
	return data;
}

MapData *ItemMap::getMapData(ItemInstance &item, Level &level)
{
	jstring id = u"map_" + String::toString(item.itemDamage);
	MapData *data = dynamic_cast<MapData *>(level.loadItemData(id, [](const jstring &s) { return std::make_unique<MapData>(s); }));
	if (data == nullptr)
	{
		item.itemDamage = level.getUniqueDataId(u"map");
		id = u"map_" + String::toString(item.itemDamage);
		data = new MapData(id);
		data->xCenter = level.xSpawn;
		data->zCenter = level.zSpawn;
		data->scale = 3;
		data->dimension = static_cast<byte_t>(level.dimension->id);
		data->markDirty();
		level.setItemData(id, data);
	}
	return data;
}

void ItemMap::updateMapData(Level &level, Entity &entity, MapData &data)
{
	if (level.dimension->id != data.dimension) return;

	constexpr short_t MAP_SIZE = 128;
	int_t scale = 1 << data.scale;
	int_t cx = data.xCenter;
	int_t cz = data.zCenter;
	int_t playerX = Mth::floor(entity.x - cx) / scale + MAP_SIZE / 2;
	int_t playerZ = Mth::floor(entity.z - cz) / scale + MAP_SIZE / 2;
	int_t radius = MAP_SIZE / scale;
	if (level.dimension->hasCeiling) radius /= 2;

	data.tick++;

	for (int_t px = playerX - radius + 1; px < playerX + radius; px++)
	{
		if ((px & 15) != (data.tick & 15)) continue;

		int_t minZ = 255;
		int_t maxZ = 0;
		double prevHeight = 0.0;

		for (int_t pz = playerZ - radius - 1; pz < playerZ + radius; pz++)
		{
			if (px < 0 || pz < -1 || px >= MAP_SIZE || pz >= MAP_SIZE) continue;

			int_t dx = px - playerX;
			int_t dz = pz - playerZ;
			bool edge = dx * dx + dz * dz > (radius - 2) * (radius - 2);
			int_t worldX = (cx / scale + px - MAP_SIZE / 2) * scale;
			int_t worldZ = (cz / scale + pz - MAP_SIZE / 2) * scale;
			int_t liquidDepth = 0;
			double height = 0.0;
			int_t blockCounts[256] = {};

			auto chunk = level.getChunkAt(worldX, worldZ);
			int_t lx = worldX & 15;
			int_t lz = worldZ & 15;
			int_t foundHeight = 0;

		if (level.dimension->hasCeiling)
		{
			int_t hash = worldX + worldZ * 231871;
			hash = hash * hash * 31287121 + hash * 11;
			if ((hash >> 20 & 1) == 0)
				blockCounts[Tile::dirt.id] += 10;
			else
				blockCounts[Tile::rock.id] += 10;
				height = 100.0;
			}
			else
			{
				for (int_t sx = 0; sx < scale; sx++)
				{
					for (int_t sz = 0; sz < scale; sz++)
					{
						int_t h = chunk->getHeightmap(sx + lx, sz + lz) + 1;
						int_t blockId = 0;
						if (h > 1)
						{
							bool done = false;
							do
							{
								done = true;
								blockId = chunk->getTile(sx + lx, h - 1, sz + lz);
								if (blockId == 0)
									done = false;
								else if (h > 0 && blockId > 0 && Tile::tiles[blockId]->material.mapColor == &MapColor::airColor)
									done = false;
								if (!done)
								{
									h--;
									blockId = chunk->getTile(sx + lx, h - 1, sz + lz);
								}
							} while (!done);

							if (blockId != 0 && Tile::tiles[blockId]->material.isLiquid())
							{
								int_t ly = h - 1;
								int_t below = 0;
								do
								{
									below = chunk->getTile(sx + lx, ly--, sz + lz);
									foundHeight++;
								} while (ly > 0 && below != 0 && Tile::tiles[below]->material.isLiquid());
							}
						}
						height += static_cast<double>(h) / (scale * scale);
						blockCounts[blockId]++;
					}
				}
			}

			foundHeight /= scale * scale;
			int_t maxCount = 0;
			int_t topBlock = 0;
			for (int_t i = 0; i < 256; i++)
			{
				if (blockCounts[i] > maxCount)
				{
					topBlock = i;
					maxCount = blockCounts[i];
				}
			}

			double heightDiff = (height - prevHeight) * 4.0 / (scale + 4) + ((px + pz & 1) - 0.5) * 0.4;
			byte_t shade = 1;
			if (heightDiff > 0.6) shade = 2;
			if (heightDiff < -0.6) shade = 0;

			int_t colorIndex = 0;
			if (topBlock > 0 && Tile::tiles[topBlock]->material.mapColor != nullptr)
			{
			const MapColor *mc = Tile::tiles[topBlock]->material.mapColor;
			if (mc == &MapColor::waterColor)
				{
					heightDiff = foundHeight * 0.1 + (px + pz & 1) * 0.2;
					shade = 1;
					if (heightDiff < 0.5) shade = 2;
					if (heightDiff > 0.9) shade = 0;
				}
				colorIndex = mc->colorIndex;
			}

			prevHeight = height;
			if (pz >= 0 && dx * dx + dz * dz < radius * radius && (!edge || (px + pz & 1) != 0))
			{
				byte_t oldColor = data.colors[px + pz * MAP_SIZE];
				byte_t newColor = static_cast<byte_t>(colorIndex * 4 + shade);
				if (oldColor != newColor)
				{
					if (minZ > pz) minZ = pz;
					if (maxZ < pz) maxZ = pz;
					data.colors[px + pz * MAP_SIZE] = newColor;
				}
			}
		}

		if (minZ <= maxZ)
			data.setDirtyColumn(px, minZ, maxZ);
	}
}

void ItemMap::onUpdate(ItemInstance &stack, Level &level, Entity &entity, int_t slot, bool isHeld)
{
	(void)slot;
	if (level.isOnline) return;
	MapData *data = getMapData(stack, level);
	if (data == nullptr) return;

	Player *player = dynamic_cast<Player *>(&entity);
	if (player != nullptr)
		data->updatePlayer(*player, stack);

	if (isHeld)
		updateMapData(level, entity, *data);
}

void ItemMap::onCreated(ItemInstance &stack, Level &level, Player &player)
{
	(void)player;
	stack.itemDamage = level.getUniqueDataId(u"map");
	jstring id = u"map_" + String::toString(stack.itemDamage);
	MapData *data = new MapData(id);
	level.setItemData(id, data);
	data->xCenter = Mth::floor(player.x);
	data->zCenter = Mth::floor(player.z);
	data->scale = 3;
	data->dimension = static_cast<byte_t>(level.dimension->id);
	data->markDirty();
}
