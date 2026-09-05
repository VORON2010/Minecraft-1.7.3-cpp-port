#include "client/renderer/entity/SpiderRenderer.h"

#include "OpenGL.h"
#include "client/model/SpiderModel.h"
#include "client/renderer/Textures.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "world/entity/monster/Spider.h"

SpiderRenderer::SpiderRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: MobRenderer(entityRenderDispatcher, std::make_shared<SpiderModel>(), 1.0f)
{
	setArmor(std::make_shared<SpiderModel>());
}

bool SpiderRenderer::prepareArmor(Mob &mobBase, int_t layer, float a)
{
	if (layer != 0 || entityRenderDispatcher.textures == nullptr)
		return false;
	Spider &spider = static_cast<Spider &>(mobBase);
	entityRenderDispatcher.textures->bind(entityRenderDispatcher.textures->loadTexture(u"/mob/spider_eyes.png"));
	float alpha = (1.0f - spider.getBrightness(1.0f)) * 0.5f;
	glEnable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0f, 1.0f, 1.0f, alpha);
	return true;
}

float SpiderRenderer::getFlipDegrees(Mob &mob)
{
	(void)mob;
	return 180.0f;
}
