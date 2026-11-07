#pragma once

#include "client/renderer/entity/EntityRenderer.h"

class FireballRenderer : public EntityRenderer
{
public:
	explicit FireballRenderer(EntityRenderDispatcher &entityRenderDispatcher, float scale = 2.0f);
	float fireballScale = 2.0f;
	void render(Entity &entity, double x, double y, double z, float rot, float a) override;
};
