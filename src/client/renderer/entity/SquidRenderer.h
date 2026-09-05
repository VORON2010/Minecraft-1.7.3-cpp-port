#pragma once

#include "client/renderer/entity/MobRenderer.h"

class SquidRenderer : public MobRenderer
{
public:
	SquidRenderer(EntityRenderDispatcher &entityRenderDispatcher);

protected:
	void setupRotations(Mob &mob, float bob, float bodyRot, float a) override;
	float getBob(Mob &mob, float a) override;
};
