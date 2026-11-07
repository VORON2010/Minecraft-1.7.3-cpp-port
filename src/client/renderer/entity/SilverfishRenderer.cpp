#include "client/renderer/entity/SilverfishRenderer.h"

#include "client/model/SilverfishModel.h"

SilverfishRenderer::SilverfishRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: MobRenderer(entityRenderDispatcher, std::make_shared<SilverfishModel>(), 0.3f)
{
}

float SilverfishRenderer::getFlipDegrees(Mob &mob)
{
	(void)mob;
	return 180.0f;
}
