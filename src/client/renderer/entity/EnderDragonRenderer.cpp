#include "client/renderer/entity/EnderDragonRenderer.h"

#include <cmath>

#include "client/Lighting.h"
#include "client/model/EnderDragonModel.h"
#include "client/renderer/Tesselator.h"
#include "java/Random.h"
#include "pc/OpenGL.h"
#include "util/Mth.h"
#include "world/entity/boss/EnderDragon.h"
#include "world/entity/item/EnderCrystal.h"

EnderDragon *EnderDragonRenderer::lastDragon = nullptr;

EnderDragonRenderer::EnderDragonRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: MobRenderer(entityRenderDispatcher, std::make_shared<EnderDragonModel>(), 0.5f)
{
	dragonModel = std::static_pointer_cast<EnderDragonModel>(model);
	armor = model; // 1.0 setDecorationModel: the glowing eyes reuse the body model
}

void EnderDragonRenderer::setupRotations(Mob &mob, float bob, float bodyRot, float a)
{
	(void)bob;
	(void)bodyRot;
	EnderDragon &dragon = static_cast<EnderDragon &>(mob);
	float yaw = static_cast<float>(dragon.getLatencyPos(7, a)[0]);
	float pitch = static_cast<float>(dragon.getLatencyPos(5, a)[1] - dragon.getLatencyPos(10, a)[1]);
	glRotatef(-yaw, 0.0f, 1.0f, 0.0f);
	glRotatef(pitch * 10.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, 1.0f);
	if (dragon.deathTime > 0)
	{
		float fall = (dragon.deathTime + a - 1.0f) / 20.0f * 1.6f;
		fall = Mth::sqrt(fall);
		if (fall > 1.0f)
			fall = 1.0f;
		glRotatef(fall * getFlipDegrees(dragon), 0.0f, 0.0f, 1.0f);
	}
}

void EnderDragonRenderer::prepareModel(Mob &mob, float time, float speed, float a)
{
	MobRenderer::prepareModel(mob, time, speed, a);
	EnderDragon &dragon = static_cast<EnderDragon &>(mob);
	dragonModel->dragon = &dragon;
	dragonModel->partialTick = a;
	if (dragon.dragonDeathTime > 0)
	{
		// 1.0 renderModel: the body dissolves into shuffle.png while it dies
		float fade = dragon.dragonDeathTime / 200.0f;
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, fade);
		bindTexture(u"/mob/enderdragon/shuffle.png");
		dragonModel->render(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f / 16.0f);
		glAlphaFunc(GL_GREATER, 0.1f);
		glDepthFunc(GL_EQUAL);
		bindTexture(u"/mob/enderdragon/ender.png");
	}
}

bool EnderDragonRenderer::prepareArmor(Mob &mob, int_t layer, float a)
{
	(void)mob;
	(void)a;
	if (layer == 1)
		glDepthFunc(GL_LEQUAL);
	if (layer != 0)
		return false;
	bindTexture(u"/mob/enderdragon/ender_eyes.png");
	glEnable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_LIGHTING);
	glDepthFunc(GL_EQUAL);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
	return true;
}

void EnderDragonRenderer::additionalRendering(Mob &mob, float a)
{
	MobRenderer::additionalRendering(mob, a);
	EnderDragon &dragon = static_cast<EnderDragon &>(mob);
	if (dragon.dragonDeathTime <= 0)
		return;
	// 1.0 renderMore: light beams burst out of the dying dragon
	Tesselator &t = Tesselator::instance;
	Lighting::turnOff();
	float progress = (dragon.dragonDeathTime + a) / 200.0f;
	float fadeOut = 0.0f;
	if (progress > 0.8f)
		fadeOut = (progress - 0.8f) / 0.2f;
	Random random(432LL);
	glDisable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_CULL_FACE);
	glDepthMask(false);
	glPushMatrix();
	glTranslatef(0.0f, -1.0f, -2.0f);
	for (int_t i = 0; i < (progress + progress * progress) / 2.0f * 60.0f; i++)
	{
		glRotatef(random.nextFloat() * 360.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(random.nextFloat() * 360.0f, 0.0f, 1.0f, 0.0f);
		glRotatef(random.nextFloat() * 360.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(random.nextFloat() * 360.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(random.nextFloat() * 360.0f, 0.0f, 1.0f, 0.0f);
		glRotatef(random.nextFloat() * 360.0f + progress * 90.0f, 0.0f, 0.0f, 1.0f);
		t.begin(GL_TRIANGLE_FAN);
		float len = random.nextFloat() * 20.0f + 5.0f + fadeOut * 10.0f;
		float wid = random.nextFloat() * 2.0f + 1.0f + fadeOut * 2.0f;
		t.color(16777215, static_cast<int_t>(255.0f * (1.0f - fadeOut)));
		t.vertex(0.0, 0.0, 0.0);
		t.color(16711935, 0);
		t.vertex(-0.866 * wid, len, -0.5f * wid);
		t.vertex(0.866 * wid, len, -0.5f * wid);
		t.vertex(0.0, len, 1.0f * wid);
		t.vertex(-0.866 * wid, len, -0.5f * wid);
		t.end();
	}
	glPopMatrix();
	glDepthMask(true);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glShadeModel(GL_FLAT);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	Lighting::turnOn();
}

void EnderDragonRenderer::render(Entity &entity, double x, double y, double z, float rot, float a)
{
	EnderDragon &dragon = static_cast<EnderDragon &>(entity);
	lastDragon = &dragon;
	MobRenderer::render(entity, x, y, z, rot, a);
	glDepthFunc(GL_LEQUAL);

	if (dragon.nearestCrystal == nullptr)
		return;
	// 1.0: the healing beam from the nearest crystal
	EnderCrystal &crystal = *dragon.nearestCrystal;
	float time = crystal.time + a;
	float bob = Mth::sin(time * 0.2f) / 2.0f + 0.5f;
	bob = (bob * bob + bob) * 0.2f;
	float dx = static_cast<float>(crystal.x - dragon.x - (dragon.xo - dragon.x) * (1.0f - a));
	float dy = static_cast<float>(bob + crystal.y - 1.0 - dragon.y - (dragon.yo - dragon.y) * (1.0f - a));
	float dz = static_cast<float>(crystal.z - dragon.z - (dragon.zo - dragon.z) * (1.0f - a));
	float horizontal = Mth::sqrt(dx * dx + dz * dz);
	float length = Mth::sqrt(dx * dx + dy * dy + dz * dz);
	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y) + 2.0f, static_cast<float>(z));
	glRotatef(static_cast<float>(-std::atan2(dz, dx)) * 180.0f / Mth::PI - 90.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(static_cast<float>(-std::atan2(horizontal, dy)) * 180.0f / Mth::PI - 90.0f, 1.0f, 0.0f, 0.0f);
	Tesselator &t = Tesselator::instance;
	Lighting::turnOff();
	glDisable(GL_CULL_FACE);
	bindTexture(u"/mob/enderdragon/beam.png");
	glShadeModel(GL_SMOOTH);
	float v0 = 0.0f - (dragon.tickCount + a) * 0.01f;
	float v1 = length / 32.0f - (dragon.tickCount + a) * 0.01f;
	t.begin(GL_TRIANGLE_STRIP);
	const int_t sides = 8;
	for (int_t i = 0; i <= sides; i++)
	{
		float sx = Mth::sin((i % sides) * Mth::PI * 2.0f / sides) * 0.75f;
		float sy = Mth::cos((i % sides) * Mth::PI * 2.0f / sides) * 0.75f;
		float u = (i % sides) * 1.0f / sides;
		t.color(0);
		t.vertexUV(sx * 0.2f, sy * 0.2f, 0.0, u, v1);
		t.color(16777215);
		t.vertexUV(sx, sy, length, u, v0);
	}
	t.end();
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_FLAT);
	Lighting::turnOn();
	glPopMatrix();
}
