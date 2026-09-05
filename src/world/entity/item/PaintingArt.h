#pragma once

#include <array>

#include "java/String.h"
#include "java/Type.h"

struct PaintingArt
{
	jstring title;
	int_t sizeX;
	int_t sizeY;
	int_t offsetX;
	int_t offsetY;

	static constexpr int_t MAX_TITLE_LENGTH = 13;
	static const std::array<PaintingArt, 25> values;

	static const PaintingArt *find(const jstring &title);
	static const PaintingArt &kebab();
};
