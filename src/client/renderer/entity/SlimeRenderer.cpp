#include "client/renderer/entity/SlimeRenderer.h"

#include "client/model/SlimeModel.h"
#include "world/entity/monster/Slime.h"

#include "OpenGL.h"

SlimeRenderer::SlimeRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: MobRenderer(entityRenderDispatcher, std::make_shared<SlimeModel>(16), 0.25f), innerModel(std::make_shared<SlimeModel>(0))
{
}

bool SlimeRenderer::prepareArmor(Mob &mob, int_t layer, float a)
{
	(void)mob;
	(void)a;
	if (layer == 0)
	{
		setArmor(innerModel);
		glEnable(GL_NORMALIZE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		return true;
	}
	if (layer == 1)
	{
		glDisable(GL_BLEND);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	return false;
}

void SlimeRenderer::scale(Mob &mobBase, float a)
{
	Slime &slime = static_cast<Slime &>(mobBase);
	int_t size = slime.getSlimeSize();
	float squish = (slime.squishOld + (slime.squish - slime.squishOld) * a) / (size * 0.5f + 1.0f);
	float invSquish = 1.0f / (squish + 1.0f);
	float scale = static_cast<float>(size);
	glScalef(invSquish * scale, 1.0f / invSquish * scale, invSquish * scale);
}
