#pragma once

#include "client/renderer/entity/MobRenderer.h"

class SilverfishRenderer : public MobRenderer
{
public:
	explicit SilverfishRenderer(EntityRenderDispatcher &entityRenderDispatcher);

protected:
	float getFlipDegrees(Mob &mob) override;
};
