#pragma once

#include "client/model/Model.h"
#include "client/model/Cube.h"

class SlimeModel : public Model
{
public:
	Cube slimeBodies = Cube(0, 0);
	Cube slimeRightEye = Cube(32, 0);
	Cube slimeLeftEye = Cube(32, 4);
	Cube slimeMouth = Cube(32, 8);
	bool hasFace = false;

	SlimeModel(int_t textureOffset);
	void render(float time, float r, float bob, float yRot, float xRot, float scale) override;
	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) override;
};
