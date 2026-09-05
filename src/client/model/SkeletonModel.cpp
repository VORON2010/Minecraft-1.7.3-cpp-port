#include "client/model/SkeletonModel.h"

SkeletonModel::SkeletonModel()
{
	float g = 0.0f;
	arm0 = Cube(40, 16);
	arm0.addBox(-1.0f, -2.0f, -1.0f, 2, 12, 2, g);
	arm0.setPos(-5.0f, 2.0f, 0.0f);
	arm1 = Cube(40, 16);
	arm1.mirror = true;
	arm1.addBox(-1.0f, -2.0f, -1.0f, 2, 12, 2, g);
	arm1.setPos(5.0f, 2.0f, 0.0f);
	leg0 = Cube(0, 16);
	leg0.addBox(-1.0f, 0.0f, -1.0f, 2, 12, 2, g);
	leg0.setPos(-2.0f, 12.0f, 0.0f);
	leg1 = Cube(0, 16);
	leg1.mirror = true;
	leg1.addBox(-1.0f, 0.0f, -1.0f, 2, 12, 2, g);
	leg1.setPos(2.0f, 12.0f, 0.0f);
}
