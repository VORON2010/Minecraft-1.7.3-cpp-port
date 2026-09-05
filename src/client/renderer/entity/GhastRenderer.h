#pragma once

#include "client/renderer/entity/MobRenderer.h"

class GhastRenderer : public MobRenderer
{
public:
	GhastRenderer(EntityRenderDispatcher &entityRenderDispatcher);

protected:
	void scale(Mob &mob, float a) override;
};
