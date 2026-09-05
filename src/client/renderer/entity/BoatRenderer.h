#pragma once

#include "client/model/BoatModel.h"
#include "client/renderer/entity/EntityRenderer.h"

class BoatRenderer : public EntityRenderer
{
private:
	BoatModel modelBoat;

public:
	explicit BoatRenderer(EntityRenderDispatcher &entityRenderDispatcher);

	void render(Entity &entity, double x, double y, double z, float rot, float a) override;
};
