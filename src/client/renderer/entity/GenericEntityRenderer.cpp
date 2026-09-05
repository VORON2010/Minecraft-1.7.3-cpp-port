#include "client/renderer/entity/GenericEntityRenderer.h"

#include "OpenGL.h"
#include "world/entity/Entity.h"

GenericEntityRenderer::GenericEntityRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: EntityRenderer(entityRenderDispatcher)
{
}


void GenericEntityRenderer::render(Entity &entity, double x, double y, double z, float rot, float a)
{
	(void)rot;
	(void)a;
	glPushMatrix();
	EntityRenderer::render(entity.bb, x - entity.xOld, y - entity.yOld, z - entity.zOld);
	glPopMatrix();
}
