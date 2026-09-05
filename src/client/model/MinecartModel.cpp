#include "client/model/MinecartModel.h"

#include <cmath>

MinecartModel::MinecartModel()
	: sideModels{Cube(0, 10), Cube(0, 0), Cube(0, 0), Cube(0, 0), Cube(0, 0), Cube(44, 10)}
{
	int_t width = 20;
	int_t wallHeight = 8;
	int_t length = 16;
	int_t yOffset = 4;

	sideModels[0].addBox(-width / 2.0f, -length / 2.0f, -1.0f, width, length, 2, 0.0f);
	sideModels[0].setPos(0.0f, yOffset, 0.0f);

	sideModels[5].addBox(-width / 2.0f + 1.0f, -length / 2.0f + 1.0f, -1.0f, width - 2, length - 2, 1, 0.0f);
	sideModels[5].setPos(0.0f, yOffset, 0.0f);

	sideModels[1].addBox(-width / 2.0f + 2.0f, -wallHeight - 1.0f, -1.0f, width - 4, wallHeight, 2, 0.0f);
	sideModels[1].setPos(-width / 2.0f + 1.0f, yOffset, 0.0f);

	sideModels[2].addBox(-width / 2.0f + 2.0f, -wallHeight - 1.0f, -1.0f, width - 4, wallHeight, 2, 0.0f);
	sideModels[2].setPos(width / 2.0f - 1.0f, yOffset, 0.0f);

	sideModels[3].addBox(-width / 2.0f + 2.0f, -wallHeight - 1.0f, -1.0f, width - 4, wallHeight, 2, 0.0f);
	sideModels[3].setPos(0.0f, yOffset, -length / 2.0f + 1.0f);

	sideModels[4].addBox(-width / 2.0f + 2.0f, -wallHeight - 1.0f, -1.0f, width - 4, wallHeight, 2, 0.0f);
	sideModels[4].setPos(0.0f, yOffset, length / 2.0f - 1.0f);

	sideModels[0].xRot = static_cast<float>(Mth::PI) * 0.5f;
	sideModels[1].yRot = static_cast<float>(Mth::PI) * 1.5f;
	sideModels[2].yRot = static_cast<float>(Mth::PI) * 0.5f;
	sideModels[3].yRot = static_cast<float>(Mth::PI);
	sideModels[5].xRot = static_cast<float>(Mth::PI) * -0.5f;
}

void MinecartModel::render(float time, float r, float bob, float yRot, float xRot, float scale)
{
	(void)time;
	(void)r;
	(void)yRot;
	(void)xRot;
	sideModels[5].y = 4.0f - bob;
	for (Cube &part : sideModels)
		part.render(scale);
}

void MinecartModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale)
{
	(void)time;
	(void)r;
	(void)bob;
	(void)yRot;
	(void)xRot;
	(void)scale;
}
