#include "world/level/MapData.h"

#include "nbt/CompoundTag.h"
#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"

MapData::MapData(const jstring &id) : MapDataBase(id)
{
}

void MapData::readFromNBT(CompoundTag &tag)
{
	dimension = tag.getByte(u"dimension");
	xCenter = tag.getInt(u"xCenter");
	zCenter = tag.getInt(u"zCenter");
	scale = tag.getByte(u"scale");
	if (scale < 0) scale = 0;
	if (scale > 4) scale = 4;

	short_t width = tag.getShort(u"width");
	short_t height = tag.getShort(u"height");
	colors = std::vector<byte_t>(16384, 0);
	if (!tag.contains(u"colors")) return;
	const auto &bytes = tag.getByteArray(u"colors");
	if (width == 128 && height == 128 && bytes.size() == 16384)
	{
		for (size_t i = 0; i < 16384; i++)
			colors[i] = bytes[i];
	}
	else
	{
		int_t xOff = (128 - width) / 2;
		int_t yOff = (128 - height) / 2;
		for (int_t y = 0; y < height && y < 128; y++)
		{
			int_t dy = y + yOff;
			if (dy < 0 || dy >= 128) continue;
			for (int_t x = 0; x < width && x < 128; x++)
			{
				int_t dx = x + xOff;
				if (dx < 0 || dx >= 128) continue;
				colors[dx + dy * 128] = bytes[x + y * width];
			}
		}
	}
}

void MapData::writeToNBT(CompoundTag &tag)
{
	tag.putByte(u"dimension", dimension);
	tag.putInt(u"xCenter", xCenter);
	tag.putInt(u"zCenter", zCenter);
	tag.putByte(u"scale", scale);
	tag.putShort(u"width", 128);
	tag.putShort(u"height", 128);
	tag.putByteArray(u"colors", std::vector<byte_t>(colors));
}

void MapData::updatePlayer(Player &player, ItemInstance &item)
{
	auto it = playerMapInfos.find(&player);
	if (it == playerMapInfos.end())
	{
		auto info = std::make_unique<MapInfo>(*this, player);
		MapInfo *ptr = info.get();
		mapInfos.push_back(std::move(info));
		playerMapInfos[&player] = ptr;
	}

	mapCoords.clear();

	for (const auto &info : mapInfos)
	{
		if (!info->player.removed )
		{
			float dx = static_cast<float>(info->player.x - xCenter) / static_cast<float>(1 << scale);
			float dz = static_cast<float>(info->player.z - zCenter) / static_cast<float>(1 << scale);
			if (dx >= -64.0f && dz >= -64.0f && dx <= 64.0f && dz <= 64.0f)
			{
				byte_t icon = 0;
				byte_t mx = static_cast<byte_t>(dx * 2.0f + 0.5f);
				byte_t mz = static_cast<byte_t>(dz * 2.0f + 0.5f);
				byte_t rot = static_cast<byte_t>(info->player.yRot * 16.0f / 360.0f + 0.5f);
				if (dimension < 0)
				{
					int_t t = tick / 10;
					rot = static_cast<byte_t>((t * t * 34187121 + t * 121) >> 15 & 15);
				}
				if (info->player.dimension == dimension)
					mapCoords.push_back(std::make_unique<MapCoord>(*this, icon, mx, mz, rot));
			}
		}
		else
		{
			playerMapInfos.erase(&info->player);
			
		}
	}
}

void MapData::setDirtyColumn(int_t x, int_t yMin, int_t yMax)
{
	markDirty();
	for (const auto &info : mapInfos)
	{
		if (info->dirtyMin[x] < 0 || info->dirtyMin[x] > yMin)
			info->dirtyMin[x] = yMin;
		if (info->dirtyMax[x] < 0 || info->dirtyMax[x] < yMax)
			info->dirtyMax[x] = yMax;
	}
}

void MapData::updateData(const std::vector<byte_t> &data)
{
	if (data.empty()) return;
	if (data[0] == 0)
	{
		int_t x = data[1] & 255;
		int_t y = data[2] & 255;
		for (size_t i = 0; i < data.size() - 3; i++)
			colors[(i + y) * 128 + x] = data[i + 3];
		markDirty();
	}
	else if (data[0] == 1)
	{
		mapCoords.clear();
		for (size_t i = 0; i < (data.size() - 1) / 3; i++)
		{
			byte_t icon = static_cast<byte_t>((data[i * 3 + 1] % 16));
			byte_t x = data[i * 3 + 2];
			byte_t z = data[i * 3 + 3];
			byte_t rot = static_cast<byte_t>(data[i * 3 + 1] / 16);
			mapCoords.push_back(std::make_unique<MapCoord>(*this, icon, x, z, rot));
		}
	}
}
