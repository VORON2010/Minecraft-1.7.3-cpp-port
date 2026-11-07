#include "client/renderer/entity/EndermanRenderer.h"

#include "pc/OpenGL.h"
#include "client/model/EndermanModel.h"
#include "client/renderer/Textures.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "world/entity/monster/Enderman.h"
#include "world/level/tile/Tile.h"

EndermanRenderer::EndermanRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: MobRenderer(entityRenderDispatcher, std::make_shared<EndermanModel>(), 0.5f)
{
	endermanModel = std::static_pointer_cast<EndermanModel>(model);
	setArmor(model);
}

void EndermanRenderer::render(Entity &entity, double x, double y, double z, float rot, float a)
{
	Enderman &enderman = static_cast<Enderman &>(entity);
	endermanModel->carrying = enderman.getCarriedTile() > 0;
	endermanModel->creepy = enderman.creepy;
	if (enderman.creepy)
	{
		double shake = 0.02;
		x += random.nextGaussian() * shake;
		z += random.nextGaussian() * shake;
	}
	MobRenderer::render(entity, x, y, z, rot, a);
}

void EndermanRenderer::additionalRendering(Mob &mob, float a)
{
	MobRenderer::additionalRendering(mob, a);
	Enderman &enderman = static_cast<Enderman &>(mob);
	int_t carried = enderman.getCarriedTile();
	if (carried <= 0 || Tile::tiles[carried] == nullptr)
		return;

	glEnable(GL_RESCALE_NORMAL);
	glPushMatrix();
	float s = 0.5f;
	glTranslatef(0.0f, 0.6875f, -0.75f);
	glRotatef(20.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
	glScalef(s, -s, s);
	float br = mob.getBrightness(a);
	glColor4f(br, br, br, 1.0f);
	bindTexture(u"/terrain.png");
	tileRenderer.renderTile(*Tile::tiles[carried], enderman.getCarriedData());
	glPopMatrix();
	glDisable(GL_RESCALE_NORMAL);
}

bool EndermanRenderer::prepareArmor(Mob &mob, int_t layer, float a)
{
	(void)mob;
	(void)a;
	if (layer != 0)
		return false;
	// glowing purple eyes: additive, full bright
	bindTexture(u"/mob/enderman_eyes.png");
	glEnable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glBlendFunc(GL_ONE, GL_ONE);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	return true;
}
