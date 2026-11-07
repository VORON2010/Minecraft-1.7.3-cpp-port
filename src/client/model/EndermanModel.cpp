#include "client/model/EndermanModel.h"

EndermanModel::EndermanModel() : HumanoidModel(0.0f, -14.0f)
{
	float yo = -14.0f;
	float g = 0.0f;
	hair = Cube(0, 16);
	hair.addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, g - 0.5f);
	hair.setPos(0.0f, 0.0f + yo, 0.0f);
	body = Cube(32, 16);
	body.addBox(-4.0f, 0.0f, -2.0f, 8, 12, 4, g);
	body.setPos(0.0f, 0.0f + yo, 0.0f);
	arm0 = Cube(56, 0);
	arm0.addBox(-1.0f, -2.0f, -1.0f, 2, 30, 2, g);
	arm0.setPos(-3.0f, 2.0f + yo, 0.0f);
	arm1 = Cube(56, 0);
	arm1.mirror = true;
	arm1.addBox(-1.0f, -2.0f, -1.0f, 2, 30, 2, g);
	arm1.setPos(5.0f, 2.0f + yo, 0.0f);
	leg0 = Cube(56, 0);
	leg0.addBox(-1.0f, 0.0f, -1.0f, 2, 30, 2, g);
	leg0.setPos(-2.0f, 12.0f + yo, 0.0f);
	leg1 = Cube(56, 0);
	leg1.mirror = true;
	leg1.addBox(-1.0f, 0.0f, -1.0f, 2, 30, 2, g);
	leg1.setPos(2.0f, 12.0f + yo, 0.0f);
}

void EndermanModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale)
{
	HumanoidModel::setupAnim(time, r, bob, yRot, xRot, scale);
	head.visible = true;
	float yo = -14.0f;
	body.xRot = 0.0f;
	body.y = yo;
	body.z = -0.0f;
	arm0.xRot *= 0.5f;
	arm1.xRot *= 0.5f;
	leg0.xRot *= 0.5f;
	leg1.xRot *= 0.5f;

	const float limit = 0.4f;
	for (Cube *c : {&arm0, &arm1, &leg0, &leg1})
	{
		if (c->xRot > limit) c->xRot = limit;
		if (c->xRot < -limit) c->xRot = -limit;
	}

	if (carrying)
	{
		arm0.xRot = -0.5f;
		arm1.xRot = -0.5f;
		arm0.zRot = 0.05f;
		arm1.zRot = -0.05f;
	}

	arm0.z = 0.0f;
	arm1.z = 0.0f;
	leg0.z = 0.0f;
	leg1.z = 0.0f;
	leg0.y = 9.0f + yo;
	leg1.y = 9.0f + yo;
	head.z = -0.0f;
	head.y = yo + 1.0f;
	hair.x = head.x;
	hair.y = head.y;
	hair.z = head.z;
	hair.xRot = head.xRot;
	hair.yRot = head.yRot;
	hair.zRot = head.zRot;
	if (creepy)
		head.y -= 5.0f;
}
