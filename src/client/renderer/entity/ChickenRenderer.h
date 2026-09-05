#pragma once

#include "client/renderer/entity/MobRenderer.h"

class ChickenRenderer : public MobRenderer
{
public:
	explicit ChickenRenderer(EntityRenderDispatcher &entityRenderDispatcher);

protected:
	float getBob(Mob &mob, float a) override;
};
