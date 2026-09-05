#pragma once

#include "client/renderer/entity/EntityRenderer.h"

class FireballRenderer : public EntityRenderer
{
public:
	explicit FireballRenderer(EntityRenderDispatcher &entityRenderDispatcher);
	void render(Entity &entity, double x, double y, double z, float rot, float a) override;
};
