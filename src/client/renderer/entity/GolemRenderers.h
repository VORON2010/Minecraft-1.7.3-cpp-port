#pragma once

#include "client/model/Cube.h"
#include "client/model/Model.h"
#include "client/renderer/entity/MobRenderer.h"
#include "client/renderer/TileRenderer.h"

// 1.0 SnowGolemModel (64x64 texture)
class SnowManModel : public Model
{
public:
	Cube head = Cube(0, 0);
	Cube rightArm = Cube(32, 0);
	Cube leftArm = Cube(32, 0);
	Cube top = Cube(0, 16);
	Cube bottom = Cube(0, 36);

	SnowManModel();
	void render(float time, float r, float bob, float yRot, float xRot, float scale) override;
	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) override;
};

// 1.0 SnowGolemRenderer: the snow golem wears a pumpkin
class SnowManRenderer : public MobRenderer
{
	std::shared_ptr<SnowManModel> golemModel;
	TileRenderer blockRenderer = TileRenderer(false, false);

public:
	explicit SnowManRenderer(EntityRenderDispatcher &dispatcher);

protected:
	void additionalRendering(Mob &mob, float a) override;
};

// 1.0 MooshroomRenderer: cow model with red mushrooms on the back and head
class MushroomCowRenderer : public MobRenderer
{
	std::shared_ptr<Model> cowModel;
	TileRenderer blockRenderer = TileRenderer(false, false);

public:
	MushroomCowRenderer(EntityRenderDispatcher &dispatcher, const std::shared_ptr<Model> &model);

protected:
	void additionalRendering(Mob &mob, float a) override;
};
