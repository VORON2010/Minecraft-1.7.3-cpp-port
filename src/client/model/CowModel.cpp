#include "client/model/CowModel.h"

CowModel::CowModel() : QuadrupedModel(12, 0.0f)
{
	head = Cube(0, 0);
	head.addBox(-4.0f, -4.0f, -6.0f, 8, 8, 6, 0.0f);
	head.setPos(0.0f, 4.0f, -8.0f);
	horn1.addBox(-4.0f, -5.0f, -4.0f, 1, 3, 1, 0.0f);
	horn1.setPos(0.0f, 3.0f, -7.0f);
	horn2.addBox(3.0f, -5.0f, -4.0f, 1, 3, 1, 0.0f);
	horn2.setPos(0.0f, 3.0f, -7.0f);
	udders.addBox(-2.0f, -3.0f, 0.0f, 4, 6, 2, 0.0f);
	udders.setPos(0.0f, 14.0f, 6.0f);
	udders.xRot = Mth::PI * 0.5f;
	body = Cube(18, 4);
	body.addBox(-6.0f, -10.0f, -7.0f, 12, 18, 10, 0.0f);
	body.setPos(0.0f, 5.0f, 2.0f);
	leg1.x -= 1.0f;
	leg2.x += 1.0f;
	leg3.x -= 1.0f;
	leg4.x += 1.0f;
	leg3.z -= 1.0f;
	leg4.z -= 1.0f;
}

void CowModel::render(float time, float r, float bob, float yRot, float xRot, float scale)
{
	QuadrupedModel::render(time, r, bob, yRot, xRot, scale);
	horn1.render(scale);
	horn2.render(scale);
	udders.render(scale);
}

void CowModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale)
{
	QuadrupedModel::setupAnim(time, r, bob, yRot, xRot, scale);
	horn1.yRot = head.yRot;
	horn1.xRot = head.xRot;
	horn2.yRot = head.yRot;
	horn2.xRot = head.xRot;
}
