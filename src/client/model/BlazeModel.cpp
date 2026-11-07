#include "client/model/BlazeModel.h"

#include "util/Mth.h"

BlazeModel::BlazeModel()
{
	for (int_t i = 0; i < 12; i++)
	{
		Cube rod(0, 16);
		rod.addBox(0.0f, 0.0f, 0.0f, 2, 8, 2);
		rods.push_back(rod);
	}
	head.addBox(-4.0f, -4.0f, -4.0f, 8, 8, 8);
}

void BlazeModel::render(float time, float r, float bob, float yRot, float xRot, float scale)
{
	setupAnim(time, r, bob, yRot, xRot, scale);
	head.render(scale);
	for (Cube &rod : rods)
		rod.render(scale);
}

void BlazeModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale)
{
	(void)time;
	(void)r;
	(void)scale;
	float angle = bob * Mth::PI * -0.1f;
	for (int_t i = 0; i < 4; i++)
	{
		rods[i].y = -2.0f + Mth::cos((i * 2 + bob) * 0.25f);
		rods[i].x = Mth::cos(angle) * 9.0f;
		rods[i].z = Mth::sin(angle) * 9.0f;
		angle++;
	}
	angle = Mth::PI / 4.0f + bob * Mth::PI * 0.03f;
	for (int_t i = 4; i < 8; i++)
	{
		rods[i].y = 2.0f + Mth::cos((i * 2 + bob) * 0.25f);
		rods[i].x = Mth::cos(angle) * 7.0f;
		rods[i].z = Mth::sin(angle) * 7.0f;
		angle++;
	}
	angle = 0.47123894f + bob * Mth::PI * -0.05f;
	for (int_t i = 8; i < 12; i++)
	{
		rods[i].y = 11.0f + Mth::cos((i * 1.5f + bob) * 0.5f);
		rods[i].x = Mth::cos(angle) * 5.0f;
		rods[i].z = Mth::sin(angle) * 5.0f;
		angle++;
	}
	head.yRot = yRot / Mth::RADDEG;
	head.xRot = xRot / Mth::RADDEG;
}
