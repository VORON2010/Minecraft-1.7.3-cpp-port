#include "client/model/GhastModel.h"
#include "java/Random.h"

GhastModel::GhastModel()
{
	int_t yOff = -16;
	body.addBox(-8.0f, -8.0f, -8.0f, 16, 16, 16);
	body.y += 24 + yOff;
	Random random(1660L);
	for (int_t i = 0; i < static_cast<int_t>(tentacles.size()); ++i)
	{
		float x = ((i % 3 - i / 3 % 2 * 0.5f + 0.25f) / 2.0f * 2.0f - 1.0f) * 5.0f;
		float z = (i / 3 / 2.0f * 2.0f - 1.0f) * 5.0f;
		int_t len = random.nextInt(7) + 8;
		tentacles[i].addBox(-1.0f, 0.0f, -1.0f, 2, len, 2);
		tentacles[i].x = x;
		tentacles[i].z = z;
		tentacles[i].y = 31 + yOff;
	}
}

void GhastModel::render(float time, float r, float bob, float yRot, float xRot, float scale)
{
	setupAnim(time, r, bob, yRot, xRot, scale);
	body.render(scale);
	for (Cube &tentacle : tentacles)
		tentacle.render(scale);
}

void GhastModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale)
{
	(void)time;
	(void)r;
	(void)yRot;
	(void)xRot;
	(void)scale;
	for (int_t i = 0; i < static_cast<int_t>(tentacles.size()); ++i)
		tentacles[i].xRot = 0.2f * Mth::sin(bob * 0.3f + i) + 0.4f;
}
