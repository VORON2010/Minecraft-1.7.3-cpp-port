#include "client/renderer/entity/TNTPrimedRenderer.h"

#include <cmath>
#include "OpenGL.h"
#include "client/renderer/TileRenderer.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "world/entity/PrimedTNT.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/TNTTile.h"

TNTPrimedRenderer::TNTPrimedRenderer(EntityRenderDispatcher &dispatcher)
	: EntityRenderer(dispatcher)
{
	shadowRadius = 0.5f;
}

void TNTPrimedRenderer::render(Entity &entity, double x, double y, double z, float rot, float a)
{
	PrimedTNT &primed = static_cast<PrimedTNT &>(entity);
	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));

	float fuseTimer = static_cast<float>(primed.fuse) - a + 1.0f;
	if (fuseTimer < 10.0f)
	{
		float scale = 1.0f - fuseTimer / 10.0f;
		if (scale < 0.0f) scale = 0.0f;
		if (scale > 1.0f) scale = 1.0f;
		scale *= scale;
		scale *= scale;
		float scaleFactor = 1.0f + scale * 0.3f;
		glScalef(scaleFactor, scaleFactor, scaleFactor);
	}

	float flashAlpha = (1.0f - fuseTimer / 100.0f) * 0.8f;

	bindTexture(u"/terrain.png");
	TileRenderer tileRenderer(false, false);
	tileRenderer.renderGuiTile(Tile::tnt, 0);

	if (primed.fuse / 5 % 2 == 0)
	{
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
		glColor4f(1.0f, 1.0f, 1.0f, flashAlpha);
		tileRenderer.renderGuiTile(Tile::tnt, 0);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
	}

	glPopMatrix();
}
