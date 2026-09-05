#pragma once

#include "client/renderer/entity/EntityRenderer.h"
#include "java/Random.h"

class EntityPainting;

class PaintingRenderer : public EntityRenderer
{
private:
	Random random;

	void renderPainting(EntityPainting &painting, int_t width, int_t height, int_t textureX, int_t textureY);
	void setTileBrightness(EntityPainting &painting, float x, float y);

public:
	explicit PaintingRenderer(EntityRenderDispatcher &entityRenderDispatcher)
		: EntityRenderer(entityRenderDispatcher) {}

	void render(Entity &entity, double x, double y, double z, float rot, float a) override;
};
