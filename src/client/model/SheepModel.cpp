#include "client/model/SheepModel.h"

SheepModel::SheepModel() : QuadrupedModel(12, 0.0f)
{
	head = Cube(0, 0);
	head.addBox(-3.0f, -4.0f, -6.0f, 6, 6, 8, 0.0f);
	head.setPos(0.0f, 6.0f, -8.0f);
	body = Cube(28, 8);
	body.addBox(-4.0f, -10.0f, -7.0f, 8, 16, 6, 0.0f);
	body.setPos(0.0f, 5.0f, 2.0f);
}
