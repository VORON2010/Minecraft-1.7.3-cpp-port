#pragma once

#include "client/renderer/entity/EntityRenderer.h"

class GenericEntityRenderer : public EntityRenderer
{
public:
	explicit GenericEntityRenderer(EntityRenderDispatcher &entityRenderDispatcher);

	void render(Entity &entity, double x, double y, double z, float rot, float a) override;
};
