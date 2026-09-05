#include "client/renderer/entity/GhastRenderer.h"

#include "client/model/GhastModel.h"
#include "world/entity/monster/Ghast.h"

#include "OpenGL.h"

GhastRenderer::GhastRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: MobRenderer(entityRenderDispatcher, std::make_shared<GhastModel>(), 0.5f)
{
}

void GhastRenderer::scale(Mob &mobBase, float a)
{
	Ghast &ghast = static_cast<Ghast &>(mobBase);
	float attack = (ghast.prevAttackCounter + (ghast.attackCounter - ghast.prevAttackCounter) * a) / 20.0f;
	if (attack < 0.0f)
		attack = 0.0f;
	attack = 1.0f / (attack * attack * attack * attack * attack * 2.0f + 1.0f);
	float scaleY = (8.0f + attack) / 2.0f;
	float scaleXZ = (8.0f + 1.0f / attack) / 2.0f;
	glScalef(scaleXZ, scaleY, scaleXZ);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}
