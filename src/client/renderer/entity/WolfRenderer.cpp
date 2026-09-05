#include "client/renderer/entity/WolfRenderer.h"

#include "client/model/WolfModel.h"
#include "world/entity/animal/Wolf.h"

WolfRenderer::WolfRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: MobRenderer(entityRenderDispatcher, std::make_shared<WolfModel>(), 0.5f)
{
}

float WolfRenderer::getBob(Mob &mobBase, float a)
{
	Wolf &wolf = static_cast<Wolf &>(mobBase);
	return wolf.getTailRotation();
}

void WolfRenderer::prepareModel(Mob &mobBase, float time, float speed, float a)
{
	Wolf &wolf = static_cast<Wolf &>(mobBase);
	if (auto wolfModel = std::dynamic_pointer_cast<WolfModel>(model))
		wolfModel->prepare(wolf, time, speed, a);
}
