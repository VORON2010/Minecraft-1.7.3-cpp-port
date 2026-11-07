#include "client/renderer/entity/LavaSlimeRenderer.h"

#include "client/model/LavaSlimeModel.h"
#include "pc/OpenGL.h"
#include "world/entity/monster/LavaSlime.h"

LavaSlimeRenderer::LavaSlimeRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: MobRenderer(entityRenderDispatcher, std::make_shared<LavaSlimeModel>(), 0.25f)
{
}

void LavaSlimeRenderer::prepareModel(Mob &mob, float time, float speed, float a)
{
	MobRenderer::prepareModel(mob, time, speed, a);
	LavaSlime &slime = static_cast<LavaSlime &>(mob);
	static_cast<LavaSlimeModel &>(*model).prepare(slime.squishOld + (slime.squish - slime.squishOld) * a);
}

void LavaSlimeRenderer::scale(Mob &mob, float a)
{
	LavaSlime &slime = static_cast<LavaSlime &>(mob);
	float size = static_cast<float>(slime.getSlimeSize());
	float squish = (slime.squishOld + (slime.squish - slime.squishOld) * a) / (size * 0.5f + 1.0f);
	float w = 1.0f / (squish + 1.0f);
	glScalef(w * size, 1.0f / w * size, w * size);
}
