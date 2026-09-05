#pragma once

#include "java/Type.h"

class MapData;

class MapCoord
{
public:
	MapData &mapData;
	byte_t icon = 0;
	byte_t x = 0;
	byte_t z = 0;
	byte_t rot = 0;

	MapCoord(MapData &mapData, byte_t icon, byte_t x, byte_t z, byte_t rot);
};
