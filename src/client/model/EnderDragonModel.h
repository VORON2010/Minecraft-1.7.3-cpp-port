#pragma once

#include "client/model/Model.h"
#include "client/model/ModelPart.h"

class EnderDragon;

class EnderDragonModel : public Model
{
	ModelPart head, neck, jaw, body, rearLeg, frontLeg, rearLegTip, frontLegTip, rearFoot, frontFoot, wing, wingTip;

public:
	EnderDragon *dragon = nullptr;
	float partialTick = 0.0f;

	EnderDragonModel();
	void render(float time, float r, float bob, float yRot, float xRot, float scale) override;

private:
	static float wrap(double rotation);
};
