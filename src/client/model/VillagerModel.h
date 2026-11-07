#pragma once

#include <vector>

#include "client/model/Model.h"
#include "client/model/Cube.h"

class VillagerModel : public Model
{
public:
	std::vector<Cube> head;
	std::vector<Cube> body;
	std::vector<Cube> arms;
	Cube rightLeg = Cube(0, 22);
	Cube leftLeg = Cube(0, 22);

	explicit VillagerModel(float g = 0.0f, float yOffset = 0.0f);

	void render(float time, float r, float bob, float yRot, float xRot, float scale) override;
	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) override;
};
