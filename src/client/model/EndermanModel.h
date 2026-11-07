#pragma once

#include "client/model/HumanoidModel.h"

class EndermanModel : public HumanoidModel
{
public:
	bool carrying = false;
	bool creepy = false;

	EndermanModel();

	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) override;
};
