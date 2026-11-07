#include "client/renderer/entity/GolemRenderers.h"

#include "client/model/QuadrupedModel.h"
#include "pc/OpenGL.h"
#include "util/Mth.h"
#include "world/entity/Mob.h"
#include "world/level/tile/MushroomTile.h"
#include "world/level/tile/PumpkinTile.h"
#include "world/level/tile/Tile.h"

SnowManModel::SnowManModel()
{
	float yOffs = 4.0f;
	float g = -0.5f;
	for (Cube *c : {&head, &rightArm, &leftArm, &top, &bottom})
		c->setTexSize(64, 64);
	head.addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, g);
	head.setPos(0.0f, yOffs, 0.0f);
	rightArm.addBox(-1.0f, 0.0f, -1.0f, 12, 2, 2, g);
	rightArm.setPos(0.0f, yOffs + 9.0f - 7.0f, 0.0f);
	leftArm.addBox(-1.0f, 0.0f, -1.0f, 12, 2, 2, g);
	leftArm.setPos(0.0f, yOffs + 9.0f - 7.0f, 0.0f);
	top.addBox(-5.0f, -10.0f, -5.0f, 10, 10, 10, g);
	top.setPos(0.0f, yOffs + 9.0f, 0.0f);
	bottom.addBox(-6.0f, -12.0f, -6.0f, 12, 12, 12, g);
	bottom.setPos(0.0f, yOffs + 20.0f, 0.0f);
}

void SnowManModel::setupAnim(float time, float r, float bob, float yRot, float xRot, float scale)
{
	(void)time; (void)r; (void)bob; (void)scale;
	head.yRot = yRot / Mth::RADDEG;
	head.xRot = xRot / Mth::RADDEG;
	top.yRot = yRot / Mth::RADDEG * 0.25f;
	float s = Mth::sin(top.yRot);
	float c = Mth::cos(top.yRot);
	rightArm.zRot = 1.0f;
	leftArm.zRot = -1.0f;
	rightArm.yRot = 0.0f + top.yRot;
	leftArm.yRot = Mth::PI + top.yRot;
	rightArm.x = c * 5.0f;
	rightArm.z = -s * 5.0f;
	leftArm.x = -c * 5.0f;
	leftArm.z = s * 5.0f;
}

void SnowManModel::render(float time, float r, float bob, float yRot, float xRot, float scale)
{
	setupAnim(time, r, bob, yRot, xRot, scale);
	top.render(scale);
	bottom.render(scale);
	head.render(scale);
	rightArm.render(scale);
	leftArm.render(scale);
}

SnowManRenderer::SnowManRenderer(EntityRenderDispatcher &dispatcher)
	: MobRenderer(dispatcher, std::make_shared<SnowManModel>(), 0.5f)
{
	golemModel = std::static_pointer_cast<SnowManModel>(model);
}

void SnowManRenderer::additionalRendering(Mob &mob, float a)
{
	(void)mob;
	(void)a;
	// 1.0: a pumpkin block on the head, rendered like a held block
	glPushMatrix();
	golemModel->head.translateTo(0.0625f);
	float s = 0.625f;
	glTranslatef(0.0f, -0.34375f, 0.0f);
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	glScalef(s, -s, s);
	bindTexture(u"/terrain.png");
	blockRenderer.renderTile(Tile::pumpkin, 0);
	glPopMatrix();
}

MushroomCowRenderer::MushroomCowRenderer(EntityRenderDispatcher &dispatcher, const std::shared_ptr<Model> &cow)
	: MobRenderer(dispatcher, cow, 0.7f), cowModel(cow)
{
}

void MushroomCowRenderer::additionalRendering(Mob &mob, float a)
{
	MobRenderer::additionalRendering(mob, a);
	if (mob.isBaby())
		return;
	bindTexture(u"/terrain.png");
	glEnable(GL_CULL_FACE);
	glPushMatrix();
	glScalef(1.0f, -1.0f, 1.0f);
	glTranslatef(0.2f, 0.4f, 0.5f);
	glRotatef(42.0f, 0.0f, 1.0f, 0.0f);
	blockRenderer.renderTile(Tile::redMushroom, 0);
	glTranslatef(0.1f, 0.0f, -0.6f);
	glRotatef(42.0f, 0.0f, 1.0f, 0.0f);
	blockRenderer.renderTile(Tile::redMushroom, 0);
	glPopMatrix();
	glPushMatrix();
	static_cast<QuadrupedModel &>(*cowModel).head.translateTo(0.0625f);
	glScalef(1.0f, -1.0f, 1.0f);
	glTranslatef(0.0f, 0.75f, -0.2f);
	glRotatef(12.0f, 0.0f, 1.0f, 0.0f);
	blockRenderer.renderTile(Tile::redMushroom, 0);
	glPopMatrix();
	glDisable(GL_CULL_FACE);
}
