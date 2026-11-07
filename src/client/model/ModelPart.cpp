#include "client/model/ModelPart.h"

#include "pc/OpenGL.h"
#include "util/Mth.h"

void ModelPart::addBox(int_t u, int_t v, float x0, float y0, float z0, int_t w, int_t h, int_t d)
{
	Cube box(u, v);
	box.setTexSize(texW, texH);
	box.mirror = mirror;
	box.addBox(x0, y0, z0, w, h, d);
	boxes.push_back(box);
}

void ModelPart::render(float scale)
{
	glPushMatrix();
	glTranslatef(x * scale, y * scale, z * scale);
	if (zRot != 0.0f)
		glRotatef(zRot * Mth::RADDEG, 0.0f, 0.0f, 1.0f);
	if (yRot != 0.0f)
		glRotatef(yRot * Mth::RADDEG, 0.0f, 1.0f, 0.0f);
	if (xRot != 0.0f)
		glRotatef(xRot * Mth::RADDEG, 1.0f, 0.0f, 0.0f);
	for (Cube &box : boxes)
		box.render(scale);
	for (ModelPart *child : children)
		child->render(scale);
	glPopMatrix();
}
