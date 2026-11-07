#include "client/model/LavaSlimeModel.h"

LavaSlimeModel::LavaSlimeModel()
{
	for (int_t i = 0; i < 8; i++)
	{
		int_t u = 0;
		int_t v = i;
		if (i == 2)
		{
			u = 24;
			v = 10;
		}
		else if (i == 3)
		{
			u = 24;
			v = 19;
		}
		Cube slice(u, v);
		slice.addBox(-4.0f, static_cast<float>(16 + i), -4.0f, 8, 1, 8);
		slices.push_back(slice);
	}
	body.addBox(-2.0f, 18.0f, -2.0f, 4, 4, 4);
}

void LavaSlimeModel::prepare(float squish)
{
	if (squish < 0.0f)
		squish = 0.0f;
	for (size_t i = 0; i < slices.size(); i++)
		slices[i].y = -(4.0f - static_cast<float>(i)) * squish * 1.7f;
}

void LavaSlimeModel::render(float time, float r, float bob, float yRot, float xRot, float scale)
{
	setupAnim(time, r, bob, yRot, xRot, scale);
	body.render(scale);
	for (Cube &slice : slices)
		slice.render(scale);
}
