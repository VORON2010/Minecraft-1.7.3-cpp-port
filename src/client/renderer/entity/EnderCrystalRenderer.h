#pragma once

#include "client/model/Cube.h"
#include "client/renderer/entity/EntityRenderer.h"

class EnderCrystalRenderer : public EntityRenderer
{
	Cube cube = Cube(32, 0);
	Cube glass = Cube(0, 0);
	Cube base = Cube(0, 16);

public:
	explicit EnderCrystalRenderer(EntityRenderDispatcher &entityRenderDispatcher);
	void render(Entity &entity, double x, double y, double z, float rot, float a) override;
};
