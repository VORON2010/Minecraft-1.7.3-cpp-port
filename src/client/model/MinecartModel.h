#pragma once

#include <array>

#include "client/model/Cube.h"
#include "client/model/Model.h"

class MinecartModel : public Model
{
private:
	std::array<Cube, 6> sideModels;

public:
	MinecartModel();

	void render(float time, float r, float bob, float yRot, float xRot, float scale) override;
	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) override;
};
