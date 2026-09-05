#pragma once

#include <memory>

#include "client/renderer/entity/MobRenderer.h"

class SlimeRenderer : public MobRenderer
{
private:
	std::shared_ptr<Model> innerModel;

public:
	SlimeRenderer(EntityRenderDispatcher &entityRenderDispatcher);

protected:
	bool prepareArmor(Mob &mob, int_t layer, float a) override;
	void scale(Mob &mob, float a) override;
};
