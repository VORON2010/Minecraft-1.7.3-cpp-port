#include "client/renderer/entity/PigRenderer.h"

#include "client/model/PigModel.h"
#include "client/renderer/Textures.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "world/entity/animal/Pig.h"

PigRenderer::PigRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: MobRenderer(entityRenderDispatcher, std::make_shared<PigModel>(), 0.7f)
{
	setArmor(std::make_shared<PigModel>(0.5f));
}

bool PigRenderer::prepareArmor(Mob &mobBase, int_t layer, float a)
{
	(void)a;
	Pig &pig = static_cast<Pig &>(mobBase);
	if (layer != 0 || !pig.isSaddled() || entityRenderDispatcher.textures == nullptr)
		return false;
	entityRenderDispatcher.textures->bind(entityRenderDispatcher.textures->loadTexture(u"/mob/saddle.png"));
	return true;
}
