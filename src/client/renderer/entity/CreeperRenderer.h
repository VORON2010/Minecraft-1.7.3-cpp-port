#pragma once

#include "client/renderer/entity/MobRenderer.h"

class CreeperRenderer : public MobRenderer
{
public:
	explicit CreeperRenderer(EntityRenderDispatcher &entityRenderDispatcher);

protected:
	void scale(Mob &mob, float a) override;
	int_t getOverlayColor(Mob &mob, float br, float a) override;
	bool prepareArmor(Mob &mob, int_t layer, float a) override;
	bool prepareArmorOverlay(Mob &mob, int_t layer, float a) override;
};
