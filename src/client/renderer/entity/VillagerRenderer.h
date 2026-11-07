#pragma once

#include "client/renderer/entity/MobRenderer.h"

class VillagerRenderer : public MobRenderer
{
public:
	explicit VillagerRenderer(EntityRenderDispatcher &entityRenderDispatcher);

protected:
	void scale(Mob &mob, float a) override;
};
