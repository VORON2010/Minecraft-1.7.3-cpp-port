#include "client/model/SlimeModel.h"

SlimeModel::SlimeModel(int_t textureOffset)
	: slimeBodies(0, textureOffset)
{
	slimeBodies.addBox(-4.0f, 16.0f, -4.0f, 8, 8, 8);
	if (textureOffset > 0)
	{
		slimeBodies = Cube(0, textureOffset);
		slimeBodies.addBox(-3.0f, 17.0f, -3.0f, 6, 6, 6);
		slimeRightEye.addBox(-3.25f, 18.0f, -3.5f, 2, 2, 2);
		slimeLeftEye.addBox(1.25f, 18.0f, -3.5f, 2, 2, 2);
		slimeMouth.addBox(0.0f, 21.0f, -3.5f, 1, 1, 1);
		hasFace = true;
	}
}

void SlimeModel::render(float time, float r, float bob, float yRot, float xRot, float scale)
{
	setupAnim(time, r, bob, yRot, xRot, scale);
	slimeBodies.render(scale);
	if (hasFace)
	{
		slimeRightEye.render(scale);
		slimeLeftEye.render(scale);
		slimeMouth.render(scale);
	}
}

void SlimeModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale)
{
	(void)time;
	(void)r;
	(void)bob;
	(void)yRot;
	(void)xRot;
	(void)scale;
}
