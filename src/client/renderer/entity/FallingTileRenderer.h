#pragma once

#include "client/renderer/TileRenderer.h"
#include "client/renderer/entity/EntityRenderer.h"

class FallingTile;

class FallingTileRenderer : public EntityRenderer
{
private:
	TileRenderer tileRenderer;

public:
	FallingTileRenderer(EntityRenderDispatcher &entityRenderDispatcher);

	void render(Entity &entity, double x, double y, double z, float rot, float a) override;
};
