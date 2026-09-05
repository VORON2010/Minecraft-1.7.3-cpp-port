#pragma once

#include "java/Type.h"

class MapColor
{
public:
	static constexpr int_t COUNT = 16;
	static MapColor *mapColorArray[COUNT];

	static MapColor airColor;
	static MapColor grassColor;
	static MapColor sandColor;
	static MapColor clothColor;
	static MapColor tntColor;
	static MapColor iceColor;
	static MapColor ironColor;
	static MapColor foliageColor;
	static MapColor snowColor;
	static MapColor clayColor;
	static MapColor dirtColor;
	static MapColor stoneColor;
	static MapColor waterColor;
	static MapColor woodColor;

	int_t colorIndex = 0;
	int_t colorValue = 0;

	MapColor(int_t index, int_t value);
};
