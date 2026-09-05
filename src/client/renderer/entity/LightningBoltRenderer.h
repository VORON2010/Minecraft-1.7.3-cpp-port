#pragma once

#include "client/renderer/entity/EntityRenderer.h"

class LightningBoltRenderer : public EntityRenderer
{
public:
	explicit LightningBoltRenderer(EntityRenderDispatcher &entityRenderDispatcher);
	void render(Entity &entity, double x, double y, double z, float rot, float a) override;
};

