#pragma once

#include "client/model/MinecartModel.h"
#include "client/renderer/TileRenderer.h"
#include "client/renderer/entity/EntityRenderer.h"

class MinecartRenderer : public EntityRenderer
{
private:
	MinecartModel modelMinecart;
	TileRenderer tileRenderer;

public:
	explicit MinecartRenderer(EntityRenderDispatcher &entityRenderDispatcher);

	void render(Entity &entity, double x, double y, double z, float rot, float a) override;
};
