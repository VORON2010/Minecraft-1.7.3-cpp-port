#include "client/model/CreeperModel.h"

CreeperModel::CreeperModel() : CreeperModel(0.0f)
{
}

CreeperModel::CreeperModel(float grow)
{
	constexpr byte_t yOffset = 4;
	head.addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, grow);
	head.setPos(0.0f, yOffset, 0.0f);
	body.addBox(-4.0f, 0.0f, -2.0f, 8, 12, 4, grow);
	body.setPos(0.0f, yOffset, 0.0f);
	leg1.addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, grow);
	leg1.setPos(-2.0f, 12 + yOffset, 4.0f);
	leg2.addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, grow);
	leg2.setPos(2.0f, 12 + yOffset, 4.0f);
	leg3.addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, grow);
	leg3.setPos(-2.0f, 12 + yOffset, -4.0f);
	leg4.addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, grow);
	leg4.setPos(2.0f, 12 + yOffset, -4.0f);
}

void CreeperModel::render(float time, float r, float bob, float yRot, float xRot, float scale)
{
	setupAnim(time, r, bob, yRot, xRot, scale);
	head.render(scale);
	body.render(scale);
	leg1.render(scale);
	leg2.render(scale);
	leg3.render(scale);
	leg4.render(scale);
}

void CreeperModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale)
{
	(void)bob;
	(void)scale;
	head.yRot = yRot / Mth::RADDEG;
	head.xRot = xRot / Mth::RADDEG;
	leg1.xRot = Mth::cos(time * 0.6662f) * 1.4f * r;
	leg2.xRot = Mth::cos(time * 0.6662f + Mth::PI) * 1.4f * r;
	leg3.xRot = Mth::cos(time * 0.6662f + Mth::PI) * 1.4f * r;
	leg4.xRot = Mth::cos(time * 0.6662f) * 1.4f * r;
}
