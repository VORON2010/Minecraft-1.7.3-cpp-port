#include "client/renderer/entity/ArrowRenderer.h"

#include "OpenGL.h"
#include "client/renderer/Tesselator.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "world/entity/projectile/EntityArrow.h"
#include "util/Mth.h"

ArrowRenderer::ArrowRenderer(EntityRenderDispatcher &entityRenderDispatcher) : EntityRenderer(entityRenderDispatcher)
{
	shadowRadius = 0.0f;
}

void ArrowRenderer::render(Entity &entity, double x, double y, double z, float rot, float a)
{
	(void)rot;
	EntityArrow &arrow = static_cast<EntityArrow &>(entity);
	if (arrow.yRotO == 0.0f && arrow.xRotO == 0.0f)
		return;

	bindTexture(u"/item/arrows.png");
	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	glRotatef(arrow.yRotO + (arrow.yRot - arrow.yRotO) * a - 90.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(arrow.xRotO + (arrow.xRot - arrow.xRotO) * a, 0.0f, 0.0f, 1.0f);
	Tesselator &t = Tesselator::instance;
	float u0 = 0.0f;
	float u1 = 0.5f;
	float v0 = 0.0f;
	float v1 = 5.0f / 32.0f;
	float v2 = 10.0f / 32.0f;
	float scale = 0.05625f;
	glEnable(GL_RESCALE_NORMAL);
	float shake = arrow.arrowShake - a;
	if (shake > 0.0f)
		glRotatef(-Mth::sin(shake * 3.0f) * shake, 0.0f, 0.0f, 1.0f);
	glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
	glScalef(scale, scale, scale);
	glTranslatef(-4.0f, 0.0f, 0.0f);
	glNormal3f(scale, 0.0f, 0.0f);
	t.begin();
	t.vertexUV(-7.0, -2.0, -2.0, 0.0, v1);
	t.vertexUV(-7.0, -2.0, 2.0, 0.15625, v1);
	t.vertexUV(-7.0, 2.0, 2.0, 0.15625, v2);
	t.vertexUV(-7.0, 2.0, -2.0, 0.0, v2);
	t.end();
	glNormal3f(-scale, 0.0f, 0.0f);
	t.begin();
	t.vertexUV(-7.0, 2.0, -2.0, 0.0, v1);
	t.vertexUV(-7.0, 2.0, 2.0, 0.15625, v1);
	t.vertexUV(-7.0, -2.0, 2.0, 0.15625, v2);
	t.vertexUV(-7.0, -2.0, -2.0, 0.0, v2);
	t.end();
	for (int_t i = 0; i < 4; ++i)
	{
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		glNormal3f(0.0f, 0.0f, scale);
		t.begin();
		t.vertexUV(-8.0, -2.0, 0.0, u0, v0);
		t.vertexUV(8.0, -2.0, 0.0, u1, v0);
		t.vertexUV(8.0, 2.0, 0.0, u1, v1);
		t.vertexUV(-8.0, 2.0, 0.0, u0, v1);
		t.end();
	}
	glDisable(GL_RESCALE_NORMAL);
	glPopMatrix();
}
