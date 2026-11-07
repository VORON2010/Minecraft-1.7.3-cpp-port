#include "client/model/VillagerModel.h"

#include "util/Mth.h"

namespace
{
	Cube part(int_t u, int_t v, float x0, float y0, float z0, int_t w, int_t h, int_t d, float g, float px, float py, float pz)
	{
		Cube cube(u, v);
		cube.setTexSize(64, 64);
		cube.addBox(x0, y0, z0, w, h, d, g);
		cube.setPos(px, py, pz);
		return cube;
	}
}

VillagerModel::VillagerModel(float g, float yOffset)
{
	head.push_back(part(0, 0, -4.0f, -10.0f, -4.0f, 8, 10, 8, g, 0.0f, yOffset, 0.0f));
	head.push_back(part(24, 0, -1.0f, -3.0f, -6.0f, 2, 4, 2, g, 0.0f, yOffset, 0.0f));
	body.push_back(part(16, 20, -4.0f, 0.0f, -3.0f, 8, 12, 6, g, 0.0f, yOffset, 0.0f));
	body.push_back(part(0, 38, -4.0f, 0.0f, -3.0f, 8, 18, 6, g + 0.5f, 0.0f, yOffset, 0.0f));
	arms.push_back(part(44, 22, -8.0f, -2.0f, -2.0f, 4, 8, 4, g, 0.0f, yOffset + 2.0f, 0.0f));
	arms.push_back(part(44, 22, 4.0f, -2.0f, -2.0f, 4, 8, 4, g, 0.0f, yOffset + 2.0f, 0.0f));
	arms.push_back(part(40, 38, -4.0f, 2.0f, -2.0f, 8, 4, 4, g, 0.0f, yOffset + 2.0f, 0.0f));
	rightLeg = part(0, 22, -2.0f, 0.0f, -2.0f, 4, 12, 4, g, -2.0f, 12.0f + yOffset, 0.0f);
	leftLeg = Cube(0, 22);
	leftLeg.setTexSize(64, 64);
	leftLeg.mirror = true;
	leftLeg.addBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, g);
	leftLeg.setPos(2.0f, 12.0f + yOffset, 0.0f);
}

void VillagerModel::render(float time, float r, float bob, float yRot, float xRot, float scale)
{
	setupAnim(time, r, bob, yRot, xRot, scale);
	for (Cube &c : head)
		c.render(scale);
	for (Cube &c : body)
		c.render(scale);
	rightLeg.render(scale);
	leftLeg.render(scale);
	for (Cube &c : arms)
		c.render(scale);
}

void VillagerModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale)
{
	(void)bob;
	(void)scale;
	for (Cube &c : head)
	{
		c.yRot = yRot / Mth::RADDEG;
		c.xRot = xRot / Mth::RADDEG;
	}
	for (Cube &c : arms)
	{
		c.y = 3.0f;
		c.z = -1.0f;
		c.xRot = -0.75f;
	}
	rightLeg.xRot = Mth::cos(time * 0.6662f) * 1.4f * r * 0.5f;
	leftLeg.xRot = Mth::cos(time * 0.6662f + Mth::PI) * 1.4f * r * 0.5f;
	rightLeg.yRot = 0.0f;
	leftLeg.yRot = 0.0f;
}
