#include "client/model/SilverfishModel.h"

#include <cmath>
#include <cstdlib>

namespace
{
	constexpr int_t BODY_SIZES[7][3] = {{3, 2, 2}, {4, 3, 2}, {6, 4, 3}, {3, 3, 3}, {2, 2, 3}, {2, 1, 2}, {1, 1, 2}};
	constexpr int_t BODY_TEXS[7][2] = {{0, 0}, {0, 4}, {0, 9}, {0, 16}, {0, 22}, {11, 0}, {13, 4}};
}

SilverfishModel::SilverfishModel()
{
	float z = -3.5f;
	for (int_t i = 0; i < 7; ++i)
	{
		Cube part(BODY_TEXS[i][0], BODY_TEXS[i][1]);
		part.addBox(BODY_SIZES[i][0] * -0.5f, 0.0f, BODY_SIZES[i][2] * -0.5f, BODY_SIZES[i][0], BODY_SIZES[i][1], BODY_SIZES[i][2]);
		part.setPos(0.0f, static_cast<float>(24 - BODY_SIZES[i][1]), z);
		bodyParts.push_back(part);
		zPlacement[i] = z;
		if (i < 6)
			z += (BODY_SIZES[i][2] + BODY_SIZES[i + 1][2]) * 0.5f;
	}

	Cube layer0(20, 0);
	layer0.addBox(-5.0f, 0.0f, BODY_SIZES[2][2] * -0.5f, 10, 8, BODY_SIZES[2][2]);
	layer0.setPos(0.0f, 16.0f, zPlacement[2]);
	Cube layer1(20, 11);
	layer1.addBox(-3.0f, 0.0f, BODY_SIZES[4][2] * -0.5f, 6, 4, BODY_SIZES[4][2]);
	layer1.setPos(0.0f, 20.0f, zPlacement[4]);
	Cube layer2(20, 18);
	layer2.addBox(-3.0f, 0.0f, BODY_SIZES[4][2] * -0.5f, 6, 5, BODY_SIZES[1][2]);
	layer2.setPos(0.0f, 19.0f, zPlacement[1]);
	bodyLayers.push_back(layer0);
	bodyLayers.push_back(layer1);
	bodyLayers.push_back(layer2);
}

void SilverfishModel::render(float time, float r, float bob, float yRot, float xRot, float scale)
{
	setupAnim(time, r, bob, yRot, xRot, scale);
	for (Cube &c : bodyParts)
		c.render(scale);
	for (Cube &c : bodyLayers)
		c.render(scale);
}

void SilverfishModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale)
{
	(void)time; (void)r; (void)yRot; (void)xRot; (void)scale;
	const float pi = 3.14159265f;
	for (int_t i = 0; i < 7; ++i)
	{
		bodyParts[i].yRot = std::cos(bob * 0.9f + i * 0.15f * pi) * pi * 0.05f * static_cast<float>(1 + std::abs(i - 2));
		bodyParts[i].x = std::sin(bob * 0.9f + i * 0.15f * pi) * pi * 0.2f * static_cast<float>(std::abs(i - 2));
	}
	bodyLayers[0].yRot = bodyParts[2].yRot;
	bodyLayers[1].yRot = bodyParts[4].yRot;
	bodyLayers[1].x = bodyParts[4].x;
	bodyLayers[2].yRot = bodyParts[1].yRot;
	bodyLayers[2].x = bodyParts[1].x;
}
