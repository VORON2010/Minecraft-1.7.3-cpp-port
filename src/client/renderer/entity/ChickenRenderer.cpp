#include "client/renderer/entity/ChickenRenderer.h"

#include <cmath>

#include "client/model/ChickenModel.h"
#include "world/entity/animal/Chicken.h"

ChickenRenderer::ChickenRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: MobRenderer(entityRenderDispatcher, std::make_shared<ChickenModel>(), 0.3f)
{
}

float ChickenRenderer::getBob(Mob &mobBase, float a)
{
	Chicken &chicken = static_cast<Chicken &>(mobBase);
	float flap = chicken.oFlapSpeed + (chicken.flap - chicken.oFlapSpeed) * a;
	float flapSpeed = chicken.oFlap + (chicken.flapSpeed - chicken.oFlap) * a;
	return (std::sin(flap) + 1.0f) * flapSpeed;
}
