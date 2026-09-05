#include "client/model/QuadrupedModel.h"

QuadrupedModel::QuadrupedModel(int_t legSize, float grow)
{
	head.addBox(-4.0f, -4.0f, -8.0f, 8, 8, 8, grow);
	head.setPos(0.0f, 18 - legSize, -6.0f);
	body.addBox(-5.0f, -10.0f, -7.0f, 10, 16, 8, grow);
	body.setPos(0.0f, 17 - legSize, 2.0f);
	leg1.addBox(-2.0f, 0.0f, -2.0f, 4, legSize, 4, grow);
	leg1.setPos(-3.0f, 24 - legSize, 7.0f);
	leg2.addBox(-2.0f, 0.0f, -2.0f, 4, legSize, 4, grow);
	leg2.setPos(3.0f, 24 - legSize, 7.0f);
	leg3.addBox(-2.0f, 0.0f, -2.0f, 4, legSize, 4, grow);
	leg3.setPos(-3.0f, 24 - legSize, -5.0f);
	leg4.addBox(-2.0f, 0.0f, -2.0f, 4, legSize, 4, grow);
	leg4.setPos(3.0f, 24 - legSize, -5.0f);
}

void QuadrupedModel::render(float time, float r, float bob, float yRot, float xRot, float scale)
{
	setupAnim(time, r, bob, yRot, xRot, scale);
	head.render(scale);
	body.render(scale);
	leg1.render(scale);
	leg2.render(scale);
	leg3.render(scale);
	leg4.render(scale);
}

void QuadrupedModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale)
{
	(void)bob;
	(void)scale;
	head.xRot = xRot / Mth::RADDEG;
	head.yRot = yRot / Mth::RADDEG;
	body.xRot = Mth::PI * 0.5f;
	leg1.xRot = Mth::cos(time * 0.6662f) * 1.4f * r;
	leg2.xRot = Mth::cos(time * 0.6662f + Mth::PI) * 1.4f * r;
	leg3.xRot = Mth::cos(time * 0.6662f + Mth::PI) * 1.4f * r;
	leg4.xRot = Mth::cos(time * 0.6662f) * 1.4f * r;
}
