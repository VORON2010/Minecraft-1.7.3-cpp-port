#pragma once

#include "client/model/Model.h"
#include "client/model/Cube.h"

class ChickenModel : public Model
{
public:
	Cube head = Cube(0, 0);
	Cube body = Cube(0, 9);
	Cube rightLeg = Cube(26, 0);
	Cube leftLeg = Cube(26, 0);
	Cube rightWing = Cube(24, 13);
	Cube leftWing = Cube(24, 13);
	Cube bill = Cube(14, 0);
	Cube chin = Cube(14, 4);

	ChickenModel();

	void render(float time, float r, float bob, float yRot, float xRot, float scale) override;
	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) override;
};
