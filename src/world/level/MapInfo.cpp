#include "world/level/MapInfo.h"

MapInfo::MapInfo(MapData &mapData, Player &player) : mapData(mapData), player(player)
{
	for (int_t i = 0; i < 128; i++)
	{
		dirtyMin[i] = 0;
		dirtyMax[i] = 127;
	}
}
