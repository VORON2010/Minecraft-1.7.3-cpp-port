#include "client/renderer/entity/BlazeRenderer.h"

#include "client/model/BlazeModel.h"

BlazeRenderer::BlazeRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: MobRenderer(entityRenderDispatcher, std::make_shared<BlazeModel>(), 0.5f)
{
}
