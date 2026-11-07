#pragma once

#include "client/model/QuadrupedModel.h"

class CowModel : public QuadrupedModel
{
private:
	Cube udders = Cube(52, 0);
	Cube horn1 = Cube(22, 0);
	Cube horn2 = Cube(22, 0);

public:
	CowModel();
	void renderHeadParts(float scale) override;
	void renderBodyParts(float scale) override;
	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) override;
};
