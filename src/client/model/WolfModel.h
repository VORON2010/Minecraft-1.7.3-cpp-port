#pragma once

#include "client/model/Model.h"
#include "client/model/Cube.h"

class Wolf;

class WolfModel : public Model
{
public:
	Cube head = Cube(0, 0);
	Cube body = Cube(18, 14);
	Cube leg1 = Cube(0, 18);
	Cube leg2 = Cube(0, 18);
	Cube leg3 = Cube(0, 18);
	Cube leg4 = Cube(0, 18);
	Cube rightEar = Cube(16, 14);
	Cube leftEar = Cube(16, 14);
	Cube snout = Cube(0, 10);
	Cube tail = Cube(9, 18);
	Cube mane = Cube(21, 0);

	WolfModel();
	void prepare(Wolf &wolf, float time, float speed, float a);
	void render(float time, float r, float bob, float yRot, float xRot, float scale) override;
	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) override;
};
