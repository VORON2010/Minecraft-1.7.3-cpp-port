#include "client/model/ChickenModel.h"

ChickenModel::ChickenModel()
{
	constexpr byte_t base = 16;
	head.addBox(-2.0f, -6.0f, -2.0f, 4, 6, 3, 0.0f);
	head.setPos(0.0f, -1 + base, -4.0f);
	bill.addBox(-2.0f, -4.0f, -4.0f, 4, 2, 2, 0.0f);
	bill.setPos(0.0f, -1 + base, -4.0f);
	chin.addBox(-1.0f, -2.0f, -3.0f, 2, 2, 2, 0.0f);
	chin.setPos(0.0f, -1 + base, -4.0f);
	body.addBox(-3.0f, -4.0f, -3.0f, 6, 8, 6, 0.0f);
	body.setPos(0.0f, base, 0.0f);
	rightLeg.addBox(-1.0f, 0.0f, -3.0f, 3, 5, 3);
	rightLeg.setPos(-2.0f, 3 + base, 1.0f);
	leftLeg.addBox(-1.0f, 0.0f, -3.0f, 3, 5, 3);
	leftLeg.setPos(1.0f, 3 + base, 1.0f);
	rightWing.addBox(0.0f, 0.0f, -3.0f, 1, 4, 6, 0.0f);
	rightWing.setPos(-4.0f, -3 + base, 0.0f);
	leftWing.addBox(-1.0f, 0.0f, -3.0f, 1, 4, 6, 0.0f);
	leftWing.setPos(4.0f, -3 + base, 0.0f);
}

void ChickenModel::render(float time, float r, float bob, float yRot, float xRot, float scale)
{
	setupAnim(time, r, bob, yRot, xRot, scale);
	head.render(scale);
	bill.render(scale);
	chin.render(scale);
	body.render(scale);
	rightLeg.render(scale);
	leftLeg.render(scale);
	rightWing.render(scale);
	leftWing.render(scale);
}

void ChickenModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale)
{
	(void)scale;
	head.xRot = -xRot / Mth::RADDEG;
	head.yRot = yRot / Mth::RADDEG;
	bill.xRot = head.xRot;
	bill.yRot = head.yRot;
	chin.xRot = head.xRot;
	chin.yRot = head.yRot;
	body.xRot = Mth::PI * 0.5f;
	rightLeg.xRot = Mth::cos(time * 0.6662f) * 1.4f * r;
	leftLeg.xRot = Mth::cos(time * 0.6662f + Mth::PI) * 1.4f * r;
	rightWing.zRot = bob;
	leftWing.zRot = -bob;
}
