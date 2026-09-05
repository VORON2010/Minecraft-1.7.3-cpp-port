#include "client/model/BoatModel.h"

BoatModel::BoatModel()
	: cubes{Cube(0, 8), Cube(0, 0), Cube(0, 0), Cube(0, 0), Cube(0, 0)}
{
	int_t w = 24;
	int_t d = 6;
	int_t h = 20;
	int_t yOff = 4;

	cubes[0].addBox(-w / 2.0f, -h / 2.0f + 2.0f, -3.0f, w, h - 4, 4, 0.0f);
	cubes[0].setPos(0.0f, static_cast<float>(yOff), 0.0f);

	cubes[1].addBox(-w / 2.0f + 2.0f, -d - 1.0f, -1.0f, w - 4, d, 2, 0.0f);
	cubes[1].setPos(-w / 2.0f + 1.0f, static_cast<float>(yOff), 0.0f);

	cubes[2].addBox(-w / 2.0f + 2.0f, -d - 1.0f, -1.0f, w - 4, d, 2, 0.0f);
	cubes[2].setPos(w / 2.0f - 1.0f, static_cast<float>(yOff), 0.0f);

	cubes[3].addBox(-w / 2.0f + 2.0f, -d - 1.0f, -1.0f, w - 4, d, 2, 0.0f);
	cubes[3].setPos(0.0f, static_cast<float>(yOff), -h / 2.0f + 1.0f);

	cubes[4].addBox(-w / 2.0f + 2.0f, -d - 1.0f, -1.0f, w - 4, d, 2, 0.0f);
	cubes[4].setPos(0.0f, static_cast<float>(yOff), h / 2.0f - 1.0f);

	cubes[0].xRot = Mth::PI * 0.5f;
	cubes[1].yRot = Mth::PI * 1.5f;
	cubes[2].yRot = Mth::PI * 0.5f;
	cubes[3].yRot = Mth::PI;
}

void BoatModel::render(float time, float r, float bob, float yRot, float xRot, float scale)
{
	(void)time;
	(void)r;
	(void)bob;
	(void)yRot;
	(void)xRot;
	for (Cube &part : cubes)
		part.render(scale);
}

void BoatModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale)
{
	(void)time;
	(void)r;
	(void)bob;
	(void)yRot;
	(void)xRot;
	(void)scale;
}
