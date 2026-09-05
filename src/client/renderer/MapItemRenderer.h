#pragma once

#include <vector>

#include "java/Type.h"

class Font;
class MapData;
class Options;
class Textures;

class MapItemRenderer
{
public:
	MapItemRenderer(Font &font, Options &options);

	void render(MapData &data, Textures &textures);

private:
	Font &font;
	Options &options;
	int_t textureId = -1;
	std::vector<int_t> pixels = std::vector<int_t>(16384, 0);
};
