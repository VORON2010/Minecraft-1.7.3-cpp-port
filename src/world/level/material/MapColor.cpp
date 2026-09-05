#include "world/level/material/MapColor.h"

MapColor *MapColor::mapColorArray[MapColor::COUNT] = {};

MapColor MapColor::airColor(0, 0);
MapColor MapColor::grassColor(1, 8368696);
MapColor MapColor::sandColor(2, 16247203);
MapColor MapColor::clothColor(3, 10987431);
MapColor MapColor::tntColor(4, 16711680);
MapColor MapColor::iceColor(5, 10526975);
MapColor MapColor::ironColor(6, 10987431);
MapColor MapColor::foliageColor(7, 31744);
MapColor MapColor::snowColor(8, 16777215);
MapColor MapColor::clayColor(9, 10791096);
MapColor MapColor::dirtColor(10, 12020271);
MapColor MapColor::stoneColor(11, 7368816);
MapColor MapColor::waterColor(12, 4210943);
MapColor MapColor::woodColor(13, 6837042);

MapColor::MapColor(int_t index, int_t value)
{
	colorIndex = index;
	colorValue = value;
	mapColorArray[index] = this;
}
