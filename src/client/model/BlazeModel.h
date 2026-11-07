#pragma once

#include <vector>

#include "client/model/Model.h"
#include "client/model/Cube.h"

class BlazeModel : public Model
{
	std::vector<Cube> rods;
	Cube head = Cube(0, 0);

public:
	BlazeModel();

	void render(float time, float r, float bob, float yRot, float xRot, float scale) override;
	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) override;
};
