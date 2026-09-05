#pragma once

#include "client/renderer/entity/EntityRenderer.h"

class ArrowRenderer : public EntityRenderer
{
public:
	explicit ArrowRenderer(EntityRenderDispatcher &entityRenderDispatcher);
	void render(Entity &entity, double x, double y, double z, float rot, float a) override;
};
