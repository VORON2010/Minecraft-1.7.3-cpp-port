#pragma once

#include <map>
#include <memory>
#include <vector>

#include "world/level/MapDataBase.h"
#include "world/level/MapCoord.h"
#include "world/level/MapInfo.h"

class ItemInstance;
class Player;

class MapData : public MapDataBase
{
public:
	int_t xCenter = 0;
	int_t zCenter = 0;
	byte_t dimension = 0;
	byte_t scale = 3;
	std::vector<byte_t> colors = std::vector<byte_t>(16384, 0);
	int_t tick = 0;

	std::vector<std::unique_ptr<MapInfo>> mapInfos;
	std::map<Player *, MapInfo *> playerMapInfos;
	std::vector<std::unique_ptr<MapCoord>> mapCoords;

	MapData(const jstring &id);

	void readFromNBT(CompoundTag &tag) override;
	void writeToNBT(CompoundTag &tag) override;

	void updatePlayer(Player &player, ItemInstance &item);
	void setDirtyColumn(int_t x, int_t yMin, int_t yMax);
	void updateData(const std::vector<byte_t> &data);
};
