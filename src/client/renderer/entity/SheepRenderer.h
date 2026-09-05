#pragma once

#include "client/renderer/entity/MobRenderer.h"

class SheepRenderer : public MobRenderer
{
public:
	explicit SheepRenderer(EntityRenderDispatcher &entityRenderDispatcher);

protected:
	bool prepareArmor(Mob &mob, int_t layer, float a) override;
};
