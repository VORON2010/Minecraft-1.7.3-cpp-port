#pragma once

#include <array>

#include "client/model/Model.h"
#include "client/model/Cube.h"

class GhastModel : public Model
{
public:
	Cube body = Cube(0, 0);
	std::array<Cube, 9> tentacles = {
		Cube(0, 0), Cube(0, 0), Cube(0, 0), Cube(0, 0), Cube(0, 0),
		Cube(0, 0), Cube(0, 0), Cube(0, 0), Cube(0, 0)
	};

	GhastModel();
	void render(float time, float r, float bob, float yRot, float xRot, float scale) override;
	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) override;
};
