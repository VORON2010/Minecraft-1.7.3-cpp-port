#pragma once

#include "client/renderer/entity/EntityRenderer.h"

class ThrownItemRenderer : public EntityRenderer
{
private:
	int_t icon = 0;

public:
	ThrownItemRenderer(EntityRenderDispatcher &entityRenderDispatcher, int_t icon);
	void render(Entity &entity, double x, double y, double z, float rot, float a) override;
};
