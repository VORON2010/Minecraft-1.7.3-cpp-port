#pragma once

#include "client/renderer/entity/MobRenderer.h"

class SpiderRenderer : public MobRenderer
{
public:
	explicit SpiderRenderer(EntityRenderDispatcher &entityRenderDispatcher);

protected:
	bool prepareArmor(Mob &mob, int_t layer, float a) override;
	float getFlipDegrees(Mob &mob) override;
};
