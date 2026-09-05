#pragma once

#include <vector>

#include "java/Type.h"

class Chunk;

class OffsettedRenderList
{
private:
	int_t x = 0, y = 0, z = 0;
	int_t layer = 0;
	std::vector<Chunk *> chunks;
	
	double xOff = 0.0, yOff = 0.0, zOff = 0.0;
	bool inited = false;
	bool rendered = false;

public:
	OffsettedRenderList() { chunks.reserve(0x10000); }

	void init(int_t x, int_t y, int_t z, int_t layer, double xOff, double yOff, double zOff);

	bool isAt(int_t x, int_t y, int_t z);

	void add(Chunk *chunk);

	void render();
	void clear();
};
