#pragma once

#include <memory>
#include <vector>

#include "client/model/Cube.h"

// 1.0 ModelPart: a pivot with several boxes (each with its own texture offset) and child parts
class ModelPart
{
	std::vector<Cube> boxes;
	std::vector<ModelPart *> children;
	int_t texW = 64;
	int_t texH = 32;

public:
	float x = 0.0f, y = 0.0f, z = 0.0f;
	float xRot = 0.0f, yRot = 0.0f, zRot = 0.0f;
	bool mirror = false;

	ModelPart &setTexSize(int_t w, int_t h)
	{
		texW = w;
		texH = h;
		return *this;
	}
	void setPos(float px, float py, float pz)
	{
		x = px;
		y = py;
		z = pz;
	}
	void addBox(int_t u, int_t v, float x0, float y0, float z0, int_t w, int_t h, int_t d);
	void addChild(ModelPart &child) { children.push_back(&child); }
	void render(float scale);
};
