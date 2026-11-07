#pragma once

#include <array>
#include <string>
#include <vector>

#include "client/MemoryTracker.h"

#include "java/Type.h"
#include "java/String.h"
#include "java/Random.h"

class Options;
class Textures;

 class Font
{
private:
	std::array<int_t, 256> charWidths;
	std::array<int_t, 32> colorCodes = {};
public:
	int_t fontTexture = 0;
	int_t fontHeight = 8;
	Random random;

	Font(Options &options, const jstring &name, Textures &textures);
	void initialize(Options &options, const jstring &name, Textures &textures);

private:
	int_t listPos = 0;
	std::vector<int_t> ib = MemoryTracker::createIntBuffer(1024);

public:
	void drawShadow(const jstring &str, int_t x, int_t y, int_t color);
	void draw(const jstring &str, int_t x, int_t y, int_t color);
	void draw(const jstring &str, int_t x, int_t y, int_t color, bool darken);

	int_t width(const jstring &str);
	void drawWordWrap(const jstring &str, int_t x, int_t y, int_t maxWidth, int_t color);
	int_t wordWrapHeight(const jstring &str, int_t maxWidth);

	// 1.0 splitAndDraw / splitAndGetHeight: wrap keeps the last colour code, line step is fontHeight
	void splitAndDraw(const jstring &str, int_t x, int_t y, int_t maxWidth, int_t color, bool darken = false);
	int_t splitAndGetHeight(const jstring &str, int_t maxWidth);
};
