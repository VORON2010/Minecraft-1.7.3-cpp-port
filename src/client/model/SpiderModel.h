#pragma once

#include "client/model/Model.h"
#include "client/model/Cube.h"

class SpiderModel : public Model
{
public:
	Cube spiderHead = Cube(32, 4);
	Cube spiderNeck = Cube(0, 0);
	Cube spiderBody = Cube(0, 12);
	Cube spiderLeg1 = Cube(18, 0);
	Cube spiderLeg2 = Cube(18, 0);
	Cube spiderLeg3 = Cube(18, 0);
	Cube spiderLeg4 = Cube(18, 0);
	Cube spiderLeg5 = Cube(18, 0);
	Cube spiderLeg6 = Cube(18, 0);
	Cube spiderLeg7 = Cube(18, 0);
	Cube spiderLeg8 = Cube(18, 0);

	SpiderModel();

	void render(float time, float r, float bob, float yRot, float xRot, float scale) override;
	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) override;
};
