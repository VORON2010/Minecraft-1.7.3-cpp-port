#include "client/renderer/entity/PaintingRenderer.h"

#include "OpenGL.h"
#include "client/renderer/Tesselator.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "util/Mth.h"
#include "world/entity/item/EntityPainting.h"
#include "world/entity/item/PaintingArt.h"
#include "world/level/Level.h"


void PaintingRenderer::render(Entity &entity, double x, double y, double z, float rot, float a)
{
	(void)a;
	EntityPainting &painting = static_cast<EntityPainting &>(entity);
	random.setSeed(187LL);
	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	glRotatef(rot, 0.0f, 1.0f, 0.0f);
	glEnable(GL_RESCALE_NORMAL);
	bindTexture(u"/art/kz.png");
	const PaintingArt &art = *painting.art;
	float scale = 1.0f / 16.0f;
	glScalef(scale, scale, scale);
	renderPainting(painting, art.sizeX, art.sizeY, art.offsetX, art.offsetY);
	glDisable(GL_RESCALE_NORMAL);
	glPopMatrix();
}

void PaintingRenderer::renderPainting(EntityPainting &painting, int_t width, int_t height, int_t textureX, int_t textureY)
{
	float left = -width / 2.0f;
	float bottom = -height / 2.0f;
	float front = -0.5f;
	float back = 0.5f;
	for (int_t ix = 0; ix < width / 16; ix++)
	{
		for (int_t iy = 0; iy < height / 16; iy++)
		{
			float x1 = left + (ix + 1) * 16;
			float x0 = left + ix * 16;
			float y1 = bottom + (iy + 1) * 16;
			float y0 = bottom + iy * 16;
			setTileBrightness(painting, (x1 + x0) / 2.0f, (y1 + y0) / 2.0f);
			float artU0 = (textureX + width - ix * 16) / 256.0f;
			float artU1 = (textureX + width - (ix + 1) * 16) / 256.0f;
			float artV0 = (textureY + height - iy * 16) / 256.0f;
			float artV1 = (textureY + height - (iy + 1) * 16) / 256.0f;
			float backU0 = 12.0f / 16.0f;
			float backU1 = 13.0f / 16.0f;
			float backV0 = 0.0f;
			float backV1 = 1.0f / 16.0f;
			float edgeU0 = 12.0f / 16.0f;
			float edgeU1 = 13.0f / 16.0f;
			float edgeV0 = 0.001953125f;
			float edgeV1 = 0.001953125f;
			float sideU0 = 385.0f / 512.0f;
			float sideU1 = 385.0f / 512.0f;
			float sideV0 = 0.0f;
			float sideV1 = 1.0f / 16.0f;
			Tesselator &t = Tesselator::instance;
			t.begin();
			t.normal(0.0f, 0.0f, -1.0f);
			t.vertexUV(x1, y0, front, artU1, artV0);
			t.vertexUV(x0, y0, front, artU0, artV0);
			t.vertexUV(x0, y1, front, artU0, artV1);
			t.vertexUV(x1, y1, front, artU1, artV1);
			t.normal(0.0f, 0.0f, 1.0f);
			t.vertexUV(x1, y1, back, backU0, backV0);
			t.vertexUV(x0, y1, back, backU1, backV0);
			t.vertexUV(x0, y0, back, backU1, backV1);
			t.vertexUV(x1, y0, back, backU0, backV1);
			t.normal(0.0f, -1.0f, 0.0f);
			t.vertexUV(x1, y1, front, edgeU0, edgeV0);
			t.vertexUV(x0, y1, front, edgeU1, edgeV0);
			t.vertexUV(x0, y1, back, edgeU1, edgeV1);
			t.vertexUV(x1, y1, back, edgeU0, edgeV1);
			t.normal(0.0f, 1.0f, 0.0f);
			t.vertexUV(x1, y0, back, edgeU0, edgeV0);
			t.vertexUV(x0, y0, back, edgeU1, edgeV0);
			t.vertexUV(x0, y0, front, edgeU1, edgeV1);
			t.vertexUV(x1, y0, front, edgeU0, edgeV1);
			t.normal(-1.0f, 0.0f, 0.0f);
			t.vertexUV(x1, y1, back, sideU1, sideV0);
			t.vertexUV(x1, y0, back, sideU1, sideV1);
			t.vertexUV(x1, y0, front, sideU0, sideV1);
			t.vertexUV(x1, y1, front, sideU0, sideV0);
			t.normal(1.0f, 0.0f, 0.0f);
			t.vertexUV(x0, y1, front, sideU1, sideV0);
			t.vertexUV(x0, y0, front, sideU1, sideV1);
			t.vertexUV(x0, y0, back, sideU0, sideV1);
			t.vertexUV(x0, y1, back, sideU0, sideV0);
			t.end();
		}
	}
}


void PaintingRenderer::setTileBrightness(EntityPainting &painting, float x, float y)
{
	int_t tileX = Mth::floor(painting.x);
	int_t tileY = Mth::floor(painting.y + y / 16.0f);
	int_t tileZ = Mth::floor(painting.z);
	if (painting.direction == 0)
		tileX = Mth::floor(painting.x + x / 16.0f);
	if (painting.direction == 1)
		tileZ = Mth::floor(painting.z - x / 16.0f);
	if (painting.direction == 2)
		tileX = Mth::floor(painting.x - x / 16.0f);
	if (painting.direction == 3)
		tileZ = Mth::floor(painting.z + x / 16.0f);
	float brightness = entityRenderDispatcher.level->getBrightness(tileX, tileY, tileZ);
	glColor3f(brightness, brightness, brightness);
}
