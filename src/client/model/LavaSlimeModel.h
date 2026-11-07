#pragma once

#include <vector>

#include "client/model/Model.h"
#include "client/model/Cube.h"

class LavaSlimeModel : public Model
{
public:
	std::vector<Cube> slices;
	Cube body = Cube(0, 16);

	LavaSlimeModel();

	void prepare(float squish);
	void render(float time, float r, float bob, float yRot, float xRot, float scale) override;
};
