#include "client/model/BookModel.h"

#include "util/Mth.h"

BookModel::BookModel()
{
	leftLid.addBox(0, 0, -6.0f, -5.0f, 0.0f, 6, 10, 0);
	rightLid.addBox(16, 0, 0.0f, -5.0f, 0.0f, 6, 10, 0);
	seam.addBox(12, 0, -1.0f, -5.0f, 0.0f, 2, 10, 0);
	leftPages.addBox(0, 10, 0.0f, -4.0f, -0.99f, 5, 8, 1);
	rightPages.addBox(12, 10, 0.0f, -4.0f, -0.01f, 5, 8, 1);
	flipPage1.addBox(24, 10, 0.0f, -4.0f, 0.0f, 5, 8, 0);
	flipPage2.addBox(24, 10, 0.0f, -4.0f, 0.0f, 5, 8, 0);
	leftLid.setPos(0.0f, 0.0f, -1.0f);
	rightLid.setPos(0.0f, 0.0f, 1.0f);
	seam.yRot = Mth::PI / 2.0f;
}

void BookModel::setupAnim(float time, float flip1, float flip2, float open)
{
	float a = (Mth::sin(time * 0.02f) * 0.1f + 1.25f) * open;
	leftLid.yRot = Mth::PI + a;
	rightLid.yRot = -a;
	leftPages.yRot = a;
	rightPages.yRot = -a;
	flipPage1.yRot = a - a * 2.0f * flip1;
	flipPage2.yRot = a - a * 2.0f * flip2;
	leftPages.x = Mth::sin(a);
	rightPages.x = Mth::sin(a);
	flipPage1.x = Mth::sin(a);
	flipPage2.x = Mth::sin(a);
}

void BookModel::render(float time, float flip1, float flip2, float open, float scale)
{
	setupAnim(time, flip1, flip2, open);
	leftLid.render(scale);
	rightLid.render(scale);
	seam.render(scale);
	leftPages.render(scale);
	rightPages.render(scale);
	flipPage1.render(scale);
	flipPage2.render(scale);
}
