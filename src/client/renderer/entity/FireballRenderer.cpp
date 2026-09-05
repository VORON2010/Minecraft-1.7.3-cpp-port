#include "client/renderer/entity/FireballRenderer.h"

#include "OpenGL.h"
#include "client/renderer/Tesselator.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/item/Items.h"

FireballRenderer::FireballRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: EntityRenderer(entityRenderDispatcher)
{
}

void FireballRenderer::render(Entity &entity, double x, double y, double z, float rot, float a)
{
	(void)entity;
	(void)rot;
	(void)a;
	bindTexture(u"/gui/items.png");
	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	glEnable(GL_RESCALE_NORMAL);
	glScalef(2.0f, 2.0f, 2.0f);
	int_t icon = Items::snowball->getIcon(ItemInstance(Items::snowball->getShiftedIndex(), 1, 0));
	float u0 = (icon % 16 * 16.0f) / 256.0f;
	float u1 = (icon % 16 * 16.0f + 16.0f) / 256.0f;
	float v0 = (icon / 16 * 16.0f) / 256.0f;
	float v1 = (icon / 16 * 16.0f + 16.0f) / 256.0f;
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
