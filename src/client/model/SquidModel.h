#pragma once

#include <array>

#include "client/model/Model.h"
#include "client/model/Cube.h"

class SquidModel : public Model
{
public:
	Cube body = Cube(0, 0);
	std::array<Cube, 8> tentacles = {
		Cube(48, 0), Cube(48, 0), Cube(48, 0), Cube(48, 0),
		Cube(48, 0), Cube(48, 0), Cube(48, 0), Cube(48, 0)
	};

	SquidModel();
	void render(float time, float r, float bob, float yRot, float xRot, float scale) override;
	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) override;
};
