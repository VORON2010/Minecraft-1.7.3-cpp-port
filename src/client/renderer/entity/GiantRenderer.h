#pragma once

#include "client/renderer/entity/HumanoidMobRenderer.h"

class GiantRenderer : public HumanoidMobRenderer
{
public:
	GiantRenderer(EntityRenderDispatcher &entityRenderDispatcher);

protected:
	void scale(Mob &mob, float a) override;
};
