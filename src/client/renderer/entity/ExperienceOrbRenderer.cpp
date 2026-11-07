#include "client/renderer/entity/ExperienceOrbRenderer.h"

#include "OpenGL.h"
#include "client/renderer/Tesselator.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "util/Mth.h"
#include "world/entity/item/ExperienceOrb.h"

ExperienceOrbRenderer::ExperienceOrbRenderer(EntityRenderDispatcher &entityRenderDispatcher) : EntityRenderer(entityRenderDispatcher)
{
	shadowRadius = 0.15f;
	shadowStrength = 0.75f;
}

void ExperienceOrbRenderer::render(Entity &entity, double x, double y, double z, float rot, float a)
{
	(void)rot;
	ExperienceOrb &orb = static_cast<ExperienceOrb &>(entity);
	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	int_t icon = orb.getIcon();
	bindTexture(u"/item/xporb.png");
	float u0 = static_cast<float>(icon % 4 * 16 + 0) / 64.0f;
	float u1 = static_cast<float>(icon % 4 * 16 + 16) / 64.0f;
	float v0 = static_cast<float>(icon / 4 * 16 + 0) / 64.0f;
	float v1 = static_cast<float>(icon / 4 * 16 + 16) / 64.0f;

	// green/yellow pulse like the original
	float br = orb.getBrightness(a);
	float time = (static_cast<float>(orb.tickCount2) + a) / 2.0f;
	int_t r = static_cast<int_t>((Mth::sin(time + 0.0f) + 1.0f) * 0.5f * 255.0f * br);
	int_t g = static_cast<int_t>(255.0f * br);
	int_t b = static_cast<int_t>((Mth::sin(time + 4.1887903f) + 1.0f) * 0.1f * 255.0f * br);

	glRotatef(180.0f - entityRenderDispatcher.playerRotY, 0.0f, 1.0f, 0.0f);
	glRotatef(-entityRenderDispatcher.playerRotX, 1.0f, 0.0f, 0.0f);
	float s = 0.3f;
	glScalef(s, s, s);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Tesselator &t = Tesselator::instance;
	t.begin();
	t.color(r, g, b, 128);
	t.normal(0.0f, 1.0f, 0.0f);
	t.vertexUV(-0.5, -0.25, 0.0, u0, v1);
	t.vertexUV(0.5, -0.25, 0.0, u1, v1);
	t.vertexUV(0.5, 0.75, 0.0, u1, v0);
	t.vertexUV(-0.5, 0.75, 0.0, u0, v0);
	t.end();
	glDisable(GL_BLEND);
	glPopMatrix();
}
