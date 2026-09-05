#pragma once

#include "client/renderer/entity/MobRenderer.h"

class Player;

class PlayerRenderer : public MobRenderer
{
private:
	std::shared_ptr<HumanoidModel> humanoidModel;
	std::shared_ptr<HumanoidModel> armorParts1;
	std::shared_ptr<HumanoidModel> armorParts2;

public:
	PlayerRenderer(EntityRenderDispatcher &entityRenderDispatcher);

	void render(Entity &entity, double x, double y, double z, float rot, float a) override;

protected:
	void scale(Mob &mob, float a) override;
	void additionalRendering(Mob &mob, float a) override;
	void setupRotations(Mob &mob, float bob, float bodyRot, float a) override;
	bool prepareArmor(Mob &mob, int_t layer, float a) override;
	void renderName(Player &player, double x, double y, double z);
	void renderLivingLabel(Player &player, const jstring &name, double x, double y, double z, int_t maxDistance);
public:
	void renderHand();
};
