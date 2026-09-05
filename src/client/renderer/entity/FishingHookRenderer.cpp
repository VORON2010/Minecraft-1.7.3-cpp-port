#include "client/renderer/entity/FishingHookRenderer.h"

#include "OpenGL.h"
#include "client/renderer/Tesselator.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "util/Mth.h"
#include "world/entity/player/Player.h"
#include "world/entity/projectile/EntityFish.h"
#include "world/phys/Vec3.h"


void FishingHookRenderer::render(Entity &entity, double x, double y, double z, float rot, float a)
{
	(void)rot;
	EntityFish &fish = static_cast<EntityFish &>(entity);
	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	glEnable(GL_RESCALE_NORMAL);
	glScalef(0.5f, 0.5f, 0.5f);
	const int_t textureX = 1;
	const int_t textureY = 2;
	bindTexture(u"/particles.png");
	Tesselator &t = Tesselator::instance;
	float u0 = (textureX * 8 + 0) / 128.0f;
	float u1 = (textureX * 8 + 8) / 128.0f;
	float v0 = (textureY * 8 + 0) / 128.0f;
	float v1 = (textureY * 8 + 8) / 128.0f;
	float size = 1.0f;
	float halfWidth = 0.5f;
	float halfHeight = 0.5f;
	glRotatef(180.0f - entityRenderDispatcher.playerRotY, 0.0f, 1.0f, 0.0f);
	glRotatef(-entityRenderDispatcher.playerRotX, 1.0f, 0.0f, 0.0f);
	t.begin();
	t.normal(0.0f, 1.0f, 0.0f);
	t.vertexUV(0.0f - halfWidth, 0.0f - halfHeight, 0.0, u0, v1);
	t.vertexUV(size - halfWidth, 0.0f - halfHeight, 0.0, u1, v1);
	t.vertexUV(size - halfWidth, 1.0f - halfHeight, 0.0, u1, v0);
	t.vertexUV(0.0f - halfWidth, 1.0f - halfHeight, 0.0, u0, v0);
	t.end();
	glDisable(GL_RESCALE_NORMAL);
	glPopMatrix();

	if (fish.angler != nullptr)
	{
		Player &angler = *fish.angler;
		float bodyRot = (angler.yRotO + (angler.yRot - angler.yRotO) * a) * Mth::PI / 180.0f;
		double bodySin = Mth::sin(bodyRot);
		double bodyCos = Mth::cos(bodyRot);
		float swing = angler.getAttackAnim(a);
		float swingSin = Mth::sin(Mth::sqrt(swing) * Mth::PI);
		Vec3 *offset = Vec3::newTemp(-0.5, 0.03, 0.8);
		offset->xRot(-(angler.xRotO + (angler.xRot - angler.xRotO) * a) * Mth::PI / 180.0f);
		offset->yRot(-(angler.yRotO + (angler.yRot - angler.yRotO) * a) * Mth::PI / 180.0f);
		offset->yRot(swingSin * 0.5f);
		offset->xRot(-swingSin * 0.7f);
		double handX = angler.xo + (angler.x - angler.xo) * a + offset->x;
		double handY = angler.yo + (angler.y - angler.yo) * a + offset->y;
		double handZ = angler.zo + (angler.z - angler.zo) * a + offset->z;
		if (entityRenderDispatcher.options->thirdPersonView)
		{
			bodyRot = (angler.yBodyRotO + (angler.yBodyRot - angler.yBodyRotO) * a) * Mth::PI / 180.0f;
			bodySin = Mth::sin(bodyRot);
			bodyCos = Mth::cos(bodyRot);
			handX = angler.xo + (angler.x - angler.xo) * a - bodyCos * 0.35 - bodySin * 0.85;
			handY = angler.yo + (angler.y - angler.yo) * a - 0.45;
			handZ = angler.zo + (angler.z - angler.zo) * a - bodySin * 0.35 + bodyCos * 0.85;
		}

		double hookX = fish.xo + (fish.x - fish.xo) * a;
		double hookY = fish.yo + (fish.y - fish.yo) * a + 0.25;
		double hookZ = fish.zo + (fish.z - fish.zo) * a;
		double dx = static_cast<float>(handX - hookX);
		double dy = static_cast<float>(handY - hookY);
		double dz = static_cast<float>(handZ - hookZ);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		t.begin(GL_LINE_STRIP);
		t.color(0);
		const int_t segments = 16;
		for (int_t i = 0; i <= segments; i++)
		{
			float f = static_cast<float>(i) / segments;
			t.vertex(x + dx * f, y + dy * (f * f + f) * 0.5 + 0.25, z + dz * f);
		}
		t.end();
		glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
	}
}
