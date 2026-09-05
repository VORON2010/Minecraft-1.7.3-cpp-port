#include "client/renderer/entity/SquidRenderer.h"

#include "client/model/SquidModel.h"
#include "world/entity/animal/Squid.h"

#include "OpenGL.h"

SquidRenderer::SquidRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: MobRenderer(entityRenderDispatcher, std::make_shared<SquidModel>(), 0.7f)
{
}

void SquidRenderer::setupRotations(Mob &mobBase, float bob, float bodyRot, float a)
{
	(void)bob;
	Squid &squid = static_cast<Squid &>(mobBase);
	float bodyXRot = squid.xBodyRotO + (squid.xBodyRot - squid.xBodyRotO) * a;
	float bodyZRot = squid.zBodyRotO + (squid.zBodyRot - squid.zBodyRotO) * a;
	glTranslatef(0.0f, 0.5f, 0.0f);
	glRotatef(180.0f - bodyRot, 0.0f, 1.0f, 0.0f);
	glRotatef(bodyXRot, 1.0f, 0.0f, 0.0f);
	glRotatef(bodyZRot, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, -1.2f, 0.0f);
}

float SquidRenderer::getBob(Mob &mobBase, float a)
{
	Squid &squid = static_cast<Squid &>(mobBase);
	return squid.oldTentacleAngle + (squid.tentacleAngle - squid.oldTentacleAngle) * a;
}
