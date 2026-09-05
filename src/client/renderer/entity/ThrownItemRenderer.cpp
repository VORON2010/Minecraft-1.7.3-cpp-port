#include "client/renderer/entity/ThrownItemRenderer.h"

#include "OpenGL.h"
#include "client/renderer/Tesselator.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"

ThrownItemRenderer::ThrownItemRenderer(EntityRenderDispatcher &entityRenderDispatcher, int_t icon)
	: EntityRenderer(entityRenderDispatcher), icon(icon)
{
	shadowRadius = 0.0f;
}

void ThrownItemRenderer::render(Entity &entity, double x, double y, double z, float rot, float a)
{
	(void)entity;
	(void)rot;
	(void)a;
	bindTexture(u"/gui/items.png");
	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	glEnable(GL_RESCALE_NORMAL);
	glScalef(0.5f, 0.5f, 0.5f);
	int_t u = (icon % 16) * 16;
	int_t v = (icon / 16) * 16;
	float u0 = u / 256.0f;
	float u1 = (u + 16.0f) / 256.0f;
	float v0 = v / 256.0f;
	float v1 = (v + 16.0f) / 256.0f;
	glRotatef(180.0f - entityRenderDispatcher.playerRotY, 0.0f, 1.0f, 0.0f);
	glRotatef(-entityRenderDispatcher.playerRotX, 1.0f, 0.0f, 0.0f);
	Tesselator &t = Tesselator::instance;
	t.begin();
	t.normal(0.0f, 1.0f, 0.0f);
	t.vertexUV(-0.5, -0.25, 0.0, u0, v1);
	t.vertexUV(0.5, -0.25, 0.0, u1, v1);
	t.vertexUV(0.5, 0.75, 0.0, u1, v0);
	t.vertexUV(-0.5, 0.75, 0.0, u0, v0);
	t.end();
	glDisable(GL_RESCALE_NORMAL);
	glPopMatrix();
}
