#pragma once

#include "client/renderer/entity/EntityRenderer.h"

class TNTPrimedRenderer : public EntityRenderer
{
public:
	TNTPrimedRenderer(EntityRenderDispatcher &dispatcher);
	void render(Entity &entity, double x, double y, double z, float rot, float a) override;
};
