#include "client/renderer/entity/VillagerRenderer.h"

#include "client/model/VillagerModel.h"
#include "pc/OpenGL.h"

VillagerRenderer::VillagerRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: MobRenderer(entityRenderDispatcher, std::make_shared<VillagerModel>(0.0f), 0.5f)
{
}

void VillagerRenderer::scale(Mob &mob, float a)
{
	(void)a;
	// 1.5.2 VillagerRenderer.scale: babies are half size with a smaller shadow
	float s = 0.9375f;
	if (mob.isBaby())
	{
		s *= 0.5f;
		shadowRadius = 0.25f;
	}
	else
		shadowRadius = 0.5f;
	glScalef(s, s, s);
}
