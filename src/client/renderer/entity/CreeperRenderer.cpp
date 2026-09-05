#include "client/renderer/entity/CreeperRenderer.h"

#include "OpenGL.h"
#include "client/model/CreeperModel.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "world/entity/monster/Creeper.h"
#include "util/Mth.h"

CreeperRenderer::CreeperRenderer(EntityRenderDispatcher &entityRenderDispatcher)
	: MobRenderer(entityRenderDispatcher, std::make_shared<CreeperModel>(), 0.5f)
{
	
	setArmor(std::make_shared<CreeperModel>(2.0f));
}


bool CreeperRenderer::prepareArmor(Mob &mobBase, int_t layer, float a)
{
	Creeper &creeper = static_cast<Creeper &>(mobBase);
	if (creeper.isPowered())
	{
		if (layer == 1)
		{
			float time = static_cast<float>(creeper.tickCount) + a;
			entityRenderDispatcher.textures->bind(entityRenderDispatcher.textures->loadTexture(u"/armor/power.png"));
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			float xo = time * 0.01f;
			float yo = time * 0.01f;
			glTranslatef(xo, yo, 0.0f);
			glMatrixMode(GL_MODELVIEW);
			glEnable(GL_BLEND);
			float c = 0.5f;
			glColor4f(c, c, c, 1.0f);
			glDisable(GL_LIGHTING);
			glBlendFunc(GL_ONE, GL_ONE);
			return true;
		}

		if (layer == 2)
		{
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			glEnable(GL_LIGHTING);
			glDisable(GL_BLEND);
		}
	}

	return false;
}



bool CreeperRenderer::prepareArmorOverlay(Mob &mobBase, int_t layer, float a)
{
	(void)mobBase;
	(void)layer;
	(void)a;
	return false;
}

void CreeperRenderer::scale(Mob &mobBase, float a)
{
	Creeper &creeper = static_cast<Creeper &>(mobBase);
	float flash = creeper.getSwelling(a);
	float scaleWobble = 1.0f + Mth::sin(flash * 100.0f) * flash * 0.01f;
	if (flash < 0.0f) flash = 0.0f;
	if (flash > 1.0f) flash = 1.0f;
	flash *= flash;
	flash *= flash;
	float xz = (1.0f + flash * 0.4f) * scaleWobble;
	float y = (1.0f + flash * 0.1f) / scaleWobble;
	glScalef(xz, y, xz);
}

int_t CreeperRenderer::getOverlayColor(Mob &mobBase, float br, float a)
{
	(void)br;
	Creeper &creeper = static_cast<Creeper &>(mobBase);
	float flash = creeper.getSwelling(a);
	if (static_cast<int_t>(flash * 10.0f) % 2 == 0)
		return 0;
	int_t alpha = static_cast<int_t>(flash * 0.2f * 255.0f);
	if (alpha < 0) alpha = 0;
	if (alpha > 255) alpha = 255;
	return (alpha << 24) | 0xFFFFFF;
}
