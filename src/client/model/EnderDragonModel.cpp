#include "client/model/EnderDragonModel.h"

#include <cmath>

#include "pc/OpenGL.h"
#include "util/Mth.h"
#include "world/entity/boss/EnderDragon.h"

// 1.0 EnderDragonModel (texture 256x256, named boxes resolved to their texture offsets)
EnderDragonModel::EnderDragonModel()
{
	for (ModelPart *p : {&head, &neck, &jaw, &body, &rearLeg, &frontLeg, &rearLegTip, &frontLegTip, &rearFoot, &frontFoot, &wing, &wingTip})
		p->setTexSize(256, 256);
	float zo = -16.0f;
	head.addBox(176, 44, -6.0f, -1.0f, -8.0f + zo, 12, 5, 16);
	head.addBox(112, 30, -8.0f, -8.0f, 6.0f + zo, 16, 16, 16);
	head.mirror = true;
	head.addBox(0, 0, -5.0f, -12.0f, 12.0f + zo, 2, 4, 6);
	head.addBox(112, 0, -5.0f, -3.0f, -6.0f + zo, 2, 2, 4);
	head.mirror = false;
	head.addBox(0, 0, 3.0f, -12.0f, 12.0f + zo, 2, 4, 6);
	head.addBox(112, 0, 3.0f, -3.0f, -6.0f + zo, 2, 2, 4);
	jaw.setPos(0.0f, 4.0f, 8.0f + zo);
	jaw.addBox(176, 65, -6.0f, 0.0f, -16.0f, 12, 4, 16);
	head.addChild(jaw);
	neck.addBox(192, 104, -5.0f, -5.0f, -5.0f, 10, 10, 10);
	neck.addBox(48, 0, -1.0f, -9.0f, -3.0f, 2, 4, 6);
	body.setPos(0.0f, 4.0f, 8.0f);
	body.addBox(0, 0, -12.0f, 0.0f, -16.0f, 24, 24, 64);
	body.addBox(220, 53, -1.0f, -6.0f, -10.0f, 2, 6, 12);
	body.addBox(220, 53, -1.0f, -6.0f, 10.0f, 2, 6, 12);
	body.addBox(220, 53, -1.0f, -6.0f, 30.0f, 2, 6, 12);
	wing.setPos(-12.0f, 5.0f, 2.0f);
	wing.addBox(112, 88, -56.0f, -4.0f, -4.0f, 56, 8, 8);
	wing.addBox(-56, 88, -56.0f, 0.0f, 2.0f, 56, 0, 56);
	wingTip.setPos(-56.0f, 0.0f, 0.0f);
	wingTip.addBox(112, 136, -56.0f, -2.0f, -2.0f, 56, 4, 4);
	wingTip.addBox(-56, 144, -56.0f, 0.0f, 2.0f, 56, 0, 56);
	wing.addChild(wingTip);
	frontLeg.setPos(-12.0f, 20.0f, 2.0f);
	frontLeg.addBox(112, 104, -4.0f, -4.0f, -4.0f, 8, 24, 8);
	frontLegTip.setPos(0.0f, 20.0f, -1.0f);
	frontLegTip.addBox(226, 138, -3.0f, -1.0f, -3.0f, 6, 24, 6);
	frontLeg.addChild(frontLegTip);
	frontFoot.setPos(0.0f, 23.0f, 0.0f);
	frontFoot.addBox(144, 104, -4.0f, 0.0f, -12.0f, 8, 4, 16);
	frontLegTip.addChild(frontFoot);
	rearLeg.setPos(-16.0f, 16.0f, 42.0f);
	rearLeg.addBox(0, 0, -8.0f, -4.0f, -8.0f, 16, 32, 16);
	rearLegTip.setPos(0.0f, 32.0f, -4.0f);
	rearLegTip.addBox(196, 0, -6.0f, -2.0f, 0.0f, 12, 32, 12);
	rearLeg.addChild(rearLegTip);
	rearFoot.setPos(0.0f, 31.0f, 4.0f);
	rearFoot.addBox(112, 0, -9.0f, 0.0f, -20.0f, 18, 6, 24);
	rearLegTip.addChild(rearFoot);
}

float EnderDragonModel::wrap(double rotation)
{
	while (rotation >= 180.0)
		rotation -= 360.0;
	while (rotation < -180.0)
		rotation += 360.0;
	return static_cast<float>(rotation);
}

void EnderDragonModel::render(float time, float r, float bob, float yRot, float xRot, float scale)
{
	(void)time;
	(void)r;
	(void)bob;
	(void)yRot;
	(void)xRot;
	if (dragon == nullptr)
		return;
	const float a = partialTick;
	const float PI = Mth::PI;
	glPushMatrix();
	float flap = dragon->oFlapTime + (dragon->flapTime - dragon->oFlapTime) * a;
	jaw.xRot = static_cast<float>(std::sin(flap * PI * 2.0f) + 1.0) * 0.2f;
	float bounce = static_cast<float>(std::sin(flap * PI * 2.0f - 1.0f) + 1.0);
	bounce = (bounce * bounce * 1.0f + bounce * 2.0f) * 0.05f;
	glTranslatef(0.0f, bounce - 2.0f, -3.0f);
	glRotatef(bounce * 2.0f, 1.0f, 0.0f, 0.0f);
	float ny = 20.0f;
	float nz = -12.0f;
	float nx = 0.0f;
	float spine = 1.5f;
	auto start = dragon->getLatencyPos(6, a);
	float bodyTurn = wrap(dragon->getLatencyPos(5, a)[0] - dragon->getLatencyPos(10, a)[0]);
	float headTurn = wrap(dragon->getLatencyPos(5, a)[0] + static_cast<double>(bodyTurn / 2.0f));
	float wave = 0.0f;
	float phase = flap * PI * 2.0f;
	for (int_t i = 0; i < 5; i++)
	{
		auto seg = dragon->getLatencyPos(5 - i, a);
		wave = static_cast<float>(std::cos(i * 0.45f + phase)) * 0.15f;
		neck.yRot = wrap(seg[0] - start[0]) * PI / 180.0f * spine;
		neck.xRot = wave + static_cast<float>(seg[1] - start[1]) * PI / 180.0f * spine * 5.0f;
		neck.zRot = -wrap(seg[0] - headTurn) * PI / 180.0f * spine;
		neck.y = ny;
		neck.z = nz;
		neck.x = nx;
		ny = static_cast<float>(ny + std::sin(neck.xRot) * 10.0);
		nz = static_cast<float>(nz - std::cos(neck.yRot) * std::cos(neck.xRot) * 10.0);
		nx = static_cast<float>(nx - std::sin(neck.yRot) * std::cos(neck.xRot) * 10.0);
		neck.render(scale);
	}
	head.y = ny;
	head.z = nz;
	head.x = nx;
	auto h = dragon->getLatencyPos(0, a);
	head.yRot = wrap(h[0] - start[0]) * PI / 180.0f;
	head.zRot = -wrap(h[0] - headTurn) * PI / 180.0f;
	head.render(scale);

	glPushMatrix();
	glTranslatef(0.0f, 1.0f, 0.0f);
	glRotatef(-bodyTurn * spine, 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, -1.0f, 0.0f);
	body.zRot = 0.0f;
	body.render(scale);
	for (int_t side = 0; side < 2; side++)
	{
		glEnable(GL_CULL_FACE);
		float w = flap * PI * 2.0f;
		wing.xRot = 0.125f - static_cast<float>(std::cos(w)) * 0.2f;
		wing.yRot = 0.25f;
		wing.zRot = static_cast<float>(std::sin(w) + 0.125) * 0.8f;
		wingTip.zRot = -static_cast<float>(std::sin(w + 2.0f) + 0.5) * 0.75f;
		rearLeg.xRot = 1.0f + bounce * 0.1f;
		rearLegTip.xRot = 0.5f + bounce * 0.1f;
		rearFoot.xRot = 0.75f + bounce * 0.1f;
		frontLeg.xRot = 1.3f + bounce * 0.1f;
		frontLegTip.xRot = -0.5f - bounce * 0.1f;
		frontFoot.xRot = 0.75f + bounce * 0.1f;
		wing.render(scale);
		frontLeg.render(scale);
		rearLeg.render(scale);
		glScalef(-1.0f, 1.0f, 1.0f);
		if (side == 0)
			glCullFace(GL_FRONT);
	}
	glPopMatrix();
	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);

	wave = -static_cast<float>(std::sin(flap * PI * 2.0f)) * 0.0f;
	phase = flap * PI * 2.0f;
	ny = 10.0f;
	nz = 60.0f;
	nx = 0.0f;
	start = dragon->getLatencyPos(11, a);
	for (int_t i = 0; i < 12; i++)
	{
		auto seg = dragon->getLatencyPos(12 + i, a);
		wave = static_cast<float>(wave + std::sin(i * 0.45f + phase) * 0.05f);
		neck.yRot = (wrap(seg[0] - start[0]) * spine + 180.0f) * PI / 180.0f;
		neck.xRot = wave + static_cast<float>(seg[1] - start[1]) * PI / 180.0f * spine * 5.0f;
		neck.zRot = wrap(seg[0] - headTurn) * PI / 180.0f * spine;
		neck.y = ny;
		neck.z = nz;
		neck.x = nx;
		ny = static_cast<float>(ny + std::sin(neck.xRot) * 10.0);
		nz = static_cast<float>(nz - std::cos(neck.yRot) * std::cos(neck.xRot) * 10.0);
		nx = static_cast<float>(nx - std::sin(neck.yRot) * std::cos(neck.xRot) * 10.0);
		neck.render(scale);
	}
	glPopMatrix();
}
