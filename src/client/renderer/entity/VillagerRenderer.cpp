#include "client/renderer/entity/VillagerRenderer.h"

#include "client/model/VillagerModel.h"
#include "pc/OpenGL.h"

VillagerRenderer::VillagerRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: MobRenderer(entityRenderDispatcher, std::make_shared<VillagerModel>(0.0f), 0.5f)
{
}

void VillagerRenderer::scale(Mob &mob, float a)
{
	(void)mob;
	(void)a;
	float s = 0.9375f;
	glScalef(s, s, s);
}
