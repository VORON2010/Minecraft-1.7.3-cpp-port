#include "client/model/SpiderModel.h"

SpiderModel::SpiderModel()
{
	constexpr byte_t yOffset = 15;
	spiderHead.addBox(-4.0f, -4.0f, -8.0f, 8, 8, 8, 0.0f);
	spiderHead.setPos(0.0f, yOffset, -3.0f);
	spiderNeck.addBox(-3.0f, -3.0f, -3.0f, 6, 6, 6, 0.0f);
	spiderNeck.setPos(0.0f, yOffset, 0.0f);
	spiderBody.addBox(-5.0f, -4.0f, -6.0f, 10, 8, 12, 0.0f);
	spiderBody.setPos(0.0f, yOffset, 9.0f);
	spiderLeg1.addBox(-15.0f, -1.0f, -1.0f, 16, 2, 2, 0.0f);
	spiderLeg1.setPos(-4.0f, yOffset, 2.0f);
	spiderLeg2.addBox(-1.0f, -1.0f, -1.0f, 16, 2, 2, 0.0f);
	spiderLeg2.setPos(4.0f, yOffset, 2.0f);
	spiderLeg3.addBox(-15.0f, -1.0f, -1.0f, 16, 2, 2, 0.0f);
	spiderLeg3.setPos(-4.0f, yOffset, 1.0f);
	spiderLeg4.addBox(-1.0f, -1.0f, -1.0f, 16, 2, 2, 0.0f);
	spiderLeg4.setPos(4.0f, yOffset, 1.0f);
	spiderLeg5.addBox(-15.0f, -1.0f, -1.0f, 16, 2, 2, 0.0f);
	spiderLeg5.setPos(-4.0f, yOffset, 0.0f);
	spiderLeg6.addBox(-1.0f, -1.0f, -1.0f, 16, 2, 2, 0.0f);
	spiderLeg6.setPos(4.0f, yOffset, 0.0f);
	spiderLeg7.addBox(-15.0f, -1.0f, -1.0f, 16, 2, 2, 0.0f);
	spiderLeg7.setPos(-4.0f, yOffset, -1.0f);
	spiderLeg8.addBox(-1.0f, -1.0f, -1.0f, 16, 2, 2, 0.0f);
	spiderLeg8.setPos(4.0f, yOffset, -1.0f);
}

void SpiderModel::render(float time, float r, float bob, float yRot, float xRot, float scale)
{
	setupAnim(time, r, bob, yRot, xRot, scale);
	spiderHead.render(scale);
	spiderNeck.render(scale);
	spiderBody.render(scale);
	spiderLeg1.render(scale);
	spiderLeg2.render(scale);
	spiderLeg3.render(scale);
	spiderLeg4.render(scale);
	spiderLeg5.render(scale);
	spiderLeg6.render(scale);
	spiderLeg7.render(scale);
	spiderLeg8.render(scale);
}

void SpiderModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale)
{
	(void)scale;
	spiderHead.yRot = yRot / Mth::RADDEG;
	spiderHead.xRot = xRot / Mth::RADDEG;
	float spread = Mth::PI * 0.25f;
	spiderLeg1.zRot = -spread;
	spiderLeg2.zRot = spread;
	spiderLeg3.zRot = -spread * 0.74f;
	spiderLeg4.zRot = spread * 0.74f;
	spiderLeg5.zRot = -spread * 0.74f;
	spiderLeg6.zRot = spread * 0.74f;
	spiderLeg7.zRot = -spread;
	spiderLeg8.zRot = spread;
	float baseYaw = Mth::PI * 0.125f;
	spiderLeg1.yRot = baseYaw * 2.0f;
	spiderLeg2.yRot = -baseYaw * 2.0f;
	spiderLeg3.yRot = baseYaw;
	spiderLeg4.yRot = -baseYaw;
	spiderLeg5.yRot = -baseYaw;
	spiderLeg6.yRot = baseYaw;
	spiderLeg7.yRot = -baseYaw * 2.0f;
	spiderLeg8.yRot = baseYaw * 2.0f;
	float y0 = -(Mth::cos(time * 0.6662f * 2.0f + 0.0f) * 0.4f) * r;
	float y1 = -(Mth::cos(time * 0.6662f * 2.0f + Mth::PI) * 0.4f) * r;
	float y2 = -(Mth::cos(time * 0.6662f * 2.0f + Mth::PI * 0.5f) * 0.4f) * r;
	float y3 = -(Mth::cos(time * 0.6662f * 2.0f + Mth::PI * 1.5f) * 0.4f) * r;
	float z0 = std::abs(Mth::sin(time * 0.6662f + 0.0f) * 0.4f) * r;
	float z1 = std::abs(Mth::sin(time * 0.6662f + Mth::PI) * 0.4f) * r;
	float z2 = std::abs(Mth::sin(time * 0.6662f + Mth::PI * 0.5f) * 0.4f) * r;
	float z3 = std::abs(Mth::sin(time * 0.6662f + Mth::PI * 1.5f) * 0.4f) * r;
	spiderLeg1.yRot += y0;
	spiderLeg2.yRot -= y0;
	spiderLeg3.yRot += y1;
	spiderLeg4.yRot -= y1;
	spiderLeg5.yRot += y2;
	spiderLeg6.yRot -= y2;
	spiderLeg7.yRot += y3;
	spiderLeg8.yRot -= y3;
	spiderLeg1.zRot += z0;
	spiderLeg2.zRot -= z0;
	spiderLeg3.zRot += z1;
	spiderLeg4.zRot -= z1;
	spiderLeg5.zRot += z2;
	spiderLeg6.zRot -= z2;
	spiderLeg7.zRot += z3;
	spiderLeg8.zRot -= z3;
}
