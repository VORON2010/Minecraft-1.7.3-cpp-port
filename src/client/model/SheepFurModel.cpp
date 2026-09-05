#include "client/model/SheepFurModel.h"

SheepFurModel::SheepFurModel() : QuadrupedModel(12, 0.0f)
{
	head = Cube(0, 0);
	head.addBox(-3.0f, -4.0f, -4.0f, 6, 6, 6, 0.6f);
	head.setPos(0.0f, 6.0f, -8.0f);
	body = Cube(28, 8);
	body.addBox(-4.0f, -10.0f, -7.0f, 8, 16, 6, 1.75f);
	body.setPos(0.0f, 5.0f, 2.0f);
	float grow = 0.5f;
	leg1 = Cube(0, 16);
	leg1.addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, grow);
	leg1.setPos(-3.0f, 12.0f, 7.0f);
	leg2 = Cube(0, 16);
	leg2.addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, grow);
	leg2.setPos(3.0f, 12.0f, 7.0f);
	leg3 = Cube(0, 16);
	leg3.addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, grow);
	leg3.setPos(-3.0f, 12.0f, -5.0f);
	leg4 = Cube(0, 16);
	leg4.addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, grow);
	leg4.setPos(3.0f, 12.0f, -5.0f);
}
