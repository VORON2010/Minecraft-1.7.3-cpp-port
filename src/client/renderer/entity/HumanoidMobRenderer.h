#pragma once

#include "client/renderer/entity/MobRenderer.h"

class HumanoidModel;

class HumanoidMobRenderer : public MobRenderer
{
private:
	std::shared_ptr<HumanoidModel> humanoidModel;
	bool zombieArms = false;
	bool hasHeldItem = false;

public:
	HumanoidMobRenderer(EntityRenderDispatcher &entityRenderDispatcher, bool zombieArms, bool hasHeldItem);

public:
	void additionalRendering(Mob &mob, float a) override;
	void setModel(const std::shared_ptr<Model> &model);
};