#pragma once

#include "client/renderer/entity/MobRenderer.h"
#include "client/renderer/TileRenderer.h"
#include "java/Random.h"

class EndermanModel;

class EndermanRenderer : public MobRenderer
{
private:
	std::shared_ptr<EndermanModel> endermanModel;
	TileRenderer tileRenderer;
	Random random;

public:
	explicit EndermanRenderer(EntityRenderDispatcher &entityRenderDispatcher);

	void render(Entity &entity, double x, double y, double z, float rot, float a) override;

protected:
	void additionalRendering(Mob &mob, float a) override;
	bool prepareArmor(Mob &mob, int_t layer, float a) override;
};
