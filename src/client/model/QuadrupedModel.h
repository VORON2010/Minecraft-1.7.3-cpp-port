#pragma once

#include "client/model/Model.h"
#include "client/model/Cube.h"

class QuadrupedModel : public Model
{
public:
	Cube head = Cube(0, 0);
	Cube body = Cube(28, 8);
	Cube leg1 = Cube(0, 16);
	Cube leg2 = Cube(0, 16);
	Cube leg3 = Cube(0, 16);
	Cube leg4 = Cube(0, 16);

	// 1.0 babyHeadHeightOffset / babyHeadOffset
	float youngYOffset = 8.0f;
	float youngZOffset = 4.0f;

	QuadrupedModel(int_t legSize, float grow);
	virtual void renderHeadParts(float scale) { head.render(scale); }
	virtual void renderBodyParts(float scale);

	void render(float time, float r, float bob, float yRot, float xRot, float scale) override;
	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) override;
};
