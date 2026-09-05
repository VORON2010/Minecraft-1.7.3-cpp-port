#pragma once

#include "client/renderer/entity/EntityRenderer.h"

class FishingHookRenderer : public EntityRenderer
{
public:
	explicit FishingHookRenderer(EntityRenderDispatcher &entityRenderDispatcher)
		: EntityRenderer(entityRenderDispatcher) {}

	void render(Entity &entity, double x, double y, double z, float rot, float a) override;
};
