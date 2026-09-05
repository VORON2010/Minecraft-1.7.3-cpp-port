#include "client/model/SquidModel.h"

#include <cmath>

SquidModel::SquidModel()
{
	int_t yoffs = -16;
	body.addBox(-6.0f, -8.0f, -6.0f, 12, 16, 12);
	body.y += 24 + yoffs;

	for (int_t i = 0; i < static_cast<int_t>(tentacles.size()); ++i)
	{
		double angle = i * Mth::PI * 2.0 / tentacles.size();
		float xo = (float)std::cos(angle) * 5.0f;
		float zo = (float)std::sin(angle) * 5.0f;
		tentacles[i].addBox(-1.0f, 0.0f, -1.0f, 2, 18, 2);
		tentacles[i].x = xo;
		tentacles[i].z = zo;
		tentacles[i].y = 31 + yoffs;
		angle = i * Mth::PI * -2.0 / tentacles.size() + Mth::PI * 0.5;
		tentacles[i].yRot = (float)angle;
	}
}

void SquidModel::render(float time, float r, float bob, float yRot, float xRot, float scale)
{
	setupAnim(time, r, bob, yRot, xRot, scale);
	body.render(scale);
	for (Cube &tentacle : tentacles)
		tentacle.render(scale);
}

void SquidModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale)
{
	(void)time;
	(void)r;
	(void)yRot;
	(void)xRot;
	(void)scale;
	for (Cube &tentacle : tentacles)
		tentacle.xRot = bob;
}
