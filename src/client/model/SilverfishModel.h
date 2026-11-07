#pragma once

#include <array>
#include <vector>

#include "client/model/Model.h"
#include "client/model/Cube.h"

class SilverfishModel : public Model
{
private:
	std::vector<Cube> bodyParts;
	std::vector<Cube> bodyLayers;
	std::array<float, 7> zPlacement = {};

public:
	SilverfishModel();

	void render(float time, float r, float bob, float yRot, float xRot, float scale) override;
	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) override;
};
