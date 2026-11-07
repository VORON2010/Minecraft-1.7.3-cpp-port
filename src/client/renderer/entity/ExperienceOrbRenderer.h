#pragma once

#include "client/renderer/entity/EntityRenderer.h"

class ExperienceOrbRenderer : public EntityRenderer
{
public:
	explicit ExperienceOrbRenderer(EntityRenderDispatcher &entityRenderDispatcher);
	void render(Entity &entity, double x, double y, double z, float rot, float a) override;
};
