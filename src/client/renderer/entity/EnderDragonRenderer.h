#pragma once

#include "client/renderer/entity/MobRenderer.h"

class EnderDragon;
class EnderDragonModel;

// 1.0 EnderDragonRenderer
class EnderDragonRenderer : public MobRenderer
{
	std::shared_ptr<EnderDragonModel> dragonModel;

public:
	// the dragon drawn last frame; the HUD reads it for the boss bar
	static EnderDragon *lastDragon;

	explicit EnderDragonRenderer(EntityRenderDispatcher &entityRenderDispatcher);
	void render(Entity &entity, double x, double y, double z, float rot, float a) override;

protected:
	void setupRotations(Mob &mob, float bob, float bodyRot, float a) override;
	void prepareModel(Mob &mob, float time, float speed, float a) override;
	bool prepareArmor(Mob &mob, int_t layer, float a) override;
	void additionalRendering(Mob &mob, float a) override;
};
