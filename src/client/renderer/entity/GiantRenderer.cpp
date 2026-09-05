#include "client/renderer/entity/GiantRenderer.h"

#include "OpenGL.h"

GiantRenderer::GiantRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: HumanoidMobRenderer(entityRenderDispatcher, true, false)
{
	shadowRadius = 3.0f;
}

void GiantRenderer::scale(Mob &mob, float a)
{
	(void)mob;
	(void)a;
	glScalef(6.0f, 6.0f, 6.0f);
}
