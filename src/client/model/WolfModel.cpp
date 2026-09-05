#include "client/model/WolfModel.h"

#include "OpenGL.h"
#include "world/entity/animal/Wolf.h"

WolfModel::WolfModel()
{
	float y = 13.5f;
	head.addBox(-3.0f, -3.0f, -2.0f, 6, 6, 4, 0.0f);
	head.setPos(-1.0f, y, -7.0f);
	body.addBox(-4.0f, -2.0f, -3.0f, 6, 9, 6, 0.0f);
	body.setPos(0.0f, 14.0f, 2.0f);
	mane.addBox(-4.0f, -3.0f, -3.0f, 8, 6, 7, 0.0f);
	mane.setPos(-1.0f, 14.0f, 2.0f);
	leg1.addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f);
	leg1.setPos(-2.5f, 16.0f, 7.0f);
	leg2.addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f);
	leg2.setPos(0.5f, 16.0f, 7.0f);
	leg3.addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f);
	leg3.setPos(-2.5f, 16.0f, -4.0f);
	leg4.addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f);
	leg4.setPos(0.5f, 16.0f, -4.0f);
	tail.addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f);
	tail.setPos(-1.0f, 12.0f, 8.0f);
	rightEar.addBox(-3.0f, -5.0f, 0.0f, 2, 2, 1, 0.0f);
	rightEar.setPos(-1.0f, y, -7.0f);
	leftEar.addBox(1.0f, -5.0f, 0.0f, 2, 2, 1, 0.0f);
	leftEar.setPos(-1.0f, y, -7.0f);
	snout.addBox(-2.0f, 0.0f, -5.0f, 3, 3, 4, 0.0f);
	snout.setPos(-0.5f, y, -7.0f);
}

void WolfModel::render(float time, float r, float bob, float yRot, float xRot, float scale)
{
	setupAnim(time, r, bob, yRot, xRot, scale);
	head.render(scale);
	body.render(scale);
	leg1.render(scale);
	leg2.render(scale);
	leg3.render(scale);
	leg4.render(scale);
	rightEar.render(scale);
	leftEar.render(scale);
	snout.render(scale);
	tail.render(scale);
	mane.render(scale);
}


void WolfModel::prepare(Wolf &wolf, float time, float speed, float a)
{
	if (wolf.isWolfAngry())
		tail.yRot = 0.0f;
	else
		tail.yRot = Mth::cos(time * 0.6662f) * 1.4f * speed;
	if (wolf.isWolfSitting())
	{
		mane.setPos(-1.0f, 16.0f, -3.0f);
		mane.xRot = Mth::PI * 0.4f;
		mane.yRot = 0.0f;
		body.setPos(0.0f, 18.0f, 0.0f);
		body.xRot = Mth::PI * 0.25f;
		tail.setPos(-1.0f, 21.0f, 6.0f);
		leg1.setPos(-2.5f, 22.0f, 2.0f);
		leg1.xRot = Mth::PI * 1.5f;
		leg2.setPos(0.5f, 22.0f, 2.0f);
		leg2.xRot = Mth::PI * 1.5f;
		leg3.xRot = Mth::PI * 1.85f;
		leg3.setPos(-2.49f, 17.0f, -4.0f);
		leg4.xRot = Mth::PI * 1.85f;
		leg4.setPos(0.51f, 17.0f, -4.0f);
	}
	else
	{
		body.setPos(0.0f, 14.0f, 2.0f);
		body.xRot = Mth::PI * 0.5f;
		mane.setPos(-1.0f, 14.0f, -3.0f);
		mane.xRot = body.xRot;
		tail.setPos(-1.0f, 12.0f, 8.0f);
		leg1.setPos(-2.5f, 16.0f, 7.0f);
		leg2.setPos(0.5f, 16.0f, 7.0f);
		leg3.setPos(-2.5f, 16.0f, -4.0f);
		leg4.setPos(0.5f, 16.0f, -4.0f);
		leg1.xRot = Mth::cos(time * 0.6662f) * 1.4f * speed;
		leg2.xRot = Mth::cos(time * 0.6662f + Mth::PI) * 1.4f * speed;
		leg3.xRot = Mth::cos(time * 0.6662f + Mth::PI) * 1.4f * speed;
		leg4.xRot = Mth::cos(time * 0.6662f) * 1.4f * speed;
	}

	float headRoll = wolf.getInterestedAngle(a) + wolf.getShakeAngle(a, 0.0f);
	head.zRot = headRoll;
	rightEar.zRot = headRoll;
	leftEar.zRot = headRoll;
	snout.zRot = headRoll;
	mane.zRot = wolf.getShakeAngle(a, -0.08f);
	body.zRot = wolf.getShakeAngle(a, -0.16f);
	tail.zRot = wolf.getShakeAngle(a, -0.2f);
	if (wolf.getWolfShaking())
	{
		float shade = wolf.getBrightness(a) * wolf.getShadingWhileShaking(a);
		glColor3f(shade, shade, shade);
	}
}

void WolfModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale)
{
	(void)time;
	(void)r;
	(void)scale;
	head.xRot = xRot / Mth::RADDEG;
	head.yRot = yRot / Mth::RADDEG;
	rightEar.yRot = head.yRot;
	rightEar.xRot = head.xRot;
	leftEar.yRot = head.yRot;
	leftEar.xRot = head.xRot;
	snout.yRot = head.yRot;
	snout.xRot = head.xRot;
	tail.xRot = bob;
}
