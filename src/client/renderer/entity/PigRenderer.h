#pragma once

#include "client/renderer/entity/MobRenderer.h"

class PigRenderer : public MobRenderer
{
public:
	explicit PigRenderer(EntityRenderDispatcher &entityRenderDispatcher);

protected:
	bool prepareArmor(Mob &mob, int_t layer, float a) override;
};
