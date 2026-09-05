#pragma once

#include "java/Type.h"

class MapData;
class Player;

class MapInfo
{
public:
	MapData &mapData;
	Player &player;
	int_t dirtyMin[128];
	int_t dirtyMax[128];
	int_t tick = 0;

	MapInfo(MapData &mapData, Player &player);
};
