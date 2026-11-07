#include "client/renderer/entity/EnderCrystalRenderer.h"

#include "pc/OpenGL.h"
#include "util/Mth.h"
#include "world/entity/item/EnderCrystal.h"

EnderCrystalRenderer::EnderCrystalRenderer(EntityRenderDispatcher &entityRenderDispatcher) : EntityRenderer(entityRenderDispatcher)
{
	shadowRadius = 0.5f;
	glass.addBox(-4.0f, -4.0f, -4.0f, 8, 8, 8);
	cube.addBox(-4.0f, -4.0f, -4.0f, 8, 8, 8);
	base.addBox(-6.0f, 0.0f, -6.0f, 12, 4, 12);
}

// 1.0 EnderCrystalRenderer + EnderCrystalModel
void EnderCrystalRenderer::render(Entity &entity, double x, double y, double z, float rot, float a)
{
	(void)rot;
	EnderCrystal &crystal = static_cast<EnderCrystal &>(entity);
	float t = static_cast<float>(crystal.time) + a;
	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	bindTexture(u"/mob/enderdragon/crystal.png");
	float bob = Mth::sin(t * 0.2f) / 2.0f + 0.5f;
	bob = bob * bob + bob;
	float spin = t * 3.0f;
	bob *= 0.2f;
	const float scale = 0.0625f;

	glPushMatrix();
	glScalef(2.0f, 2.0f, 2.0f);
	glTranslatef(0.0f, -0.5f, 0.0f);
	base.render(scale);
	glRotatef(spin, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, 0.8f + bob, 0.0f);
	glRotatef(60.0f, 0.7071f, 0.0f, 0.7071f);
	glass.render(scale);
	float shrink = 0.875f;
	glScalef(shrink, shrink, shrink);
	glRotatef(60.0f, 0.7071f, 0.0f, 0.7071f);
	glRotatef(spin, 0.0f, 1.0f, 0.0f);
	glass.render(scale);
	glScalef(shrink, shrink, shrink);
	glRotatef(60.0f, 0.7071f, 0.0f, 0.7071f);
	glRotatef(spin, 0.0f, 1.0f, 0.0f);
	cube.render(scale);
	glPopMatrix();

	glPopMatrix();
}
