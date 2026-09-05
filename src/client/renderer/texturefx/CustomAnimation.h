#pragma once

#include <vector>

#include "client/renderer/texturefx/TextureFX.h"
#include "java/String.h"

class Textures;

class CustomAnimation : public TextureFX
{
private:
	int_t frame = 0;
	int_t numFrames = 0;
	std::vector<byte_t> source;
	std::vector<byte_t> temp;
	int_t minScrollDelay = -1;
	int_t maxScrollDelay = -1;
	int_t timer = -1;
	bool scrolling = false;

public:
	CustomAnimation(Textures &textures, int_t tileNumber, int_t tileImage, int_t tileSize,
		const jstring &name, int_t minScrollDelay, int_t maxScrollDelay);

	void onTick() override;
};
