#pragma once

#include "client/renderer/entity/MobRenderer.h"

class LavaSlimeRenderer : public MobRenderer
{
public:
	explicit LavaSlimeRenderer(EntityRenderDispatcher &entityRenderDispatcher);

protected:
	void prepareModel(Mob &mob, float time, float speed, float a) override;
	void scale(Mob &mob, float a) override;
};
