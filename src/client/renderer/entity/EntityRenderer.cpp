#include "client/renderer/entity/EntityRenderer.h"

#include "client/renderer/entity/EntityRenderDispatcher.h"

#include "client/renderer/Tesselator.h"
#include <algorithm>
#include "world/level/tile/Tile.h"
#include "world/level/tile/FireTile.h"

EntityRenderer::EntityRenderer(EntityRenderDispatcher &entityRenderDispatcher) : entityRenderDispatcher(entityRenderDispatcher)
{

}

void EntityRenderer::bindTexture(const jstring &resourceName)
{
	Textures &t = *entityRenderDispatcher.textures;
	t.bind(t.loadTexture(resourceName));
}

void EntityRenderer::bindTexture(const jstring &urlTexture, const jstring &backupTexture)
{
	Textures &t = *entityRenderDispatcher.textures;
	if (!urlTexture.empty())
	{
		int_t id = t.loadHttpTexture(urlTexture, &backupTexture);
		if (id >= 0)
		{
			t.bind(id);
			return;
		}
	}

	bindTexture(backupTexture);
}

void EntityRenderer::renderFlame(Entity &e, double x, double y, double z, float a)
{
	(void)a;
	glDisable(GL_LIGHTING);
	int_t fireTexture = Tile::fire.tex;
	int_t uIndex = (fireTexture & 15) << 4;
	int_t vIndex = fireTexture & 240;
	float u0 = uIndex / 256.0f;
	float u1 = (uIndex + 15.99f) / 256.0f;
	float v0 = vIndex / 256.0f;
	float v1 = (vIndex + 15.99f) / 256.0f;
	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	float scale = e.bbWidth * 1.4f;
	glScalef(scale, scale, scale);
	bindTexture(u"/terrain.png");
	Tesselator &t = Tesselator::instance;
	float halfWidth = 0.5f;
	float offsetX = 0.0f;
	float height = e.bbHeight / scale;
	float yOffset = static_cast<float>(e.y - e.bb.y0);
	glRotatef(-entityRenderDispatcher.playerRotY, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, -0.3f + static_cast<int_t>(height) * 0.02f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	float depth = 0.0f;
	int_t layer = 0;
	t.begin();
	while (height > 0.0f)
	{
		float layerU0 = u0;
		float layerU1 = u1;
		float layerV0 = (layer % 2 == 0) ? v0 : (vIndex + 16) / 256.0f;
		float layerV1 = (layer % 2 == 0) ? v1 : (vIndex + 31.99f) / 256.0f;
		if ((layer / 2) % 2 == 0)
			std::swap(layerU0, layerU1);
		t.vertexUV(halfWidth - offsetX, 0.0f - yOffset, depth, layerU1, layerV1);
		t.vertexUV(-halfWidth - offsetX, 0.0f - yOffset, depth, layerU0, layerV1);
		t.vertexUV(-halfWidth - offsetX, 1.4f - yOffset, depth, layerU0, layerV0);
		t.vertexUV(halfWidth - offsetX, 1.4f - yOffset, depth, layerU1, layerV0);
		height -= 0.45f;
		yOffset -= 0.45f;
		halfWidth *= 0.9f;
		depth += 0.03f;
		layer++;
	}
	t.end();
	glPopMatrix();
	glEnable(GL_LIGHTING);
}

void EntityRenderer::renderShadow(Entity &e, double x, double y, double z, float pow, float a)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Textures &textures = *entityRenderDispatcher.textures;
	textures.bind(textures.loadTexture(u"%clamp%/misc/shadow.png"));

	Level &level = getLevel();
	
	glDepthMask(false);
	float r = shadowRadius;

	double ex = e.xOld + (e.x - e.xOld) * a;
	double ey = e.yOld + (e.y - e.yOld) * a + e.getShadowHeightOffs();
	double ez = e.zOld + (e.z - e.zOld) * a;

	int_t x0 = Mth::floor(ex - r);
	int_t x1 = Mth::floor(ex + r);
	int_t y0 = Mth::floor(ey - r);
	int_t y1 = Mth::floor(ey);
	int_t z0 = Mth::floor(ez - r);
	int_t z1 = Mth::floor(ez + r);

	double xo = x - ex;
	double yo = y - ey;
	double zo = z - ez;

	Tesselator &tt = Tesselator::instance;
	tt.begin();
	for (int_t xt = x0; xt <= x1; xt++)
	{
		for (int_t yt = y0; yt <= y1; yt++)
		{
			for (int_t zt = z0; zt <= z1; zt++)
			{
				int_t t = level.getTile(xt, yt - 1, zt);
				if (t > 0 && level.getRawBrightness(xt, yt, zt) > 3)
					renderTileShadow(*Tile::tiles[t], x, y + e.getShadowHeightOffs(), z, xt, yt, zt, pow, r, xo, yo + e.getShadowHeightOffs(), zo);
			}
		}
	}
	tt.end();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);
	glDepthMask(true);
}

Level &EntityRenderer::getLevel()
{
	return *entityRenderDispatcher.level;
}

void EntityRenderer::renderTileShadow(Tile &tt, double x, double y, double z, int_t xt, int_t yt, int_t zt, float pow, float r, double xo, double yo, double zo)
{
	Tesselator &t = Tesselator::instance;
	if (!tt.isCubeShaped()) return;

	double a = (pow - (y - (yt + yo)) / 2.0) * 0.5 * getLevel().getBrightness(xt, yt, zt);
	if (a < 0.0) return;
	if (a > 1.0) a = 1.0;
	t.color(1.0f, 1.0f, 1.0f, (float)a);
	
	double x0 = xt + tt.xx0 + xo;
	double x1 = xt + tt.xx1 + xo;
	double y0 = yt + tt.yy0 + yo + (1.0 / 64.0);
	double z0 = zt + tt.zz0 + zo;
	double z1 = zt + tt.zz1 + zo;
	
	float u0 = static_cast<float>((x - x0) / 2.0 / r + 0.5);
	float u1 = static_cast<float>((x - x1) / 2.0 / r + 0.5);
	float v0 = static_cast<float>((z - z0) / 2.0 / r + 0.5);
	float v1 = static_cast<float>((z - z1) / 2.0 / r + 0.5);
	
	t.vertexUV(x0, y0, z0, u0, v0);
	t.vertexUV(x0, y0, z1, u0, v1);
	t.vertexUV(x1, y0, z1, u1, v1);
	t.vertexUV(x1, y0, z0, u1, v0);
}

void EntityRenderer::render(AABB &bb, double xo, double yo, double zo)
{
	glDisable(GL_TEXTURE_2D);
	Tesselator &t = Tesselator::instance;
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	t.begin();
	t.offset(xo, yo, zo);
	t.normal(0.0f, 0.0f, -1.0f);
	t.vertex(bb.x0, bb.y1, bb.z0);
	t.vertex(bb.x1, bb.y1, bb.z0);
	t.vertex(bb.x1, bb.y0, bb.z0);
	t.vertex(bb.x0, bb.y0, bb.z0);
	
	t.normal(0.0f, 0.0f, 1.0f);
	t.vertex(bb.x0, bb.y0, bb.z1);
	t.vertex(bb.x1, bb.y0, bb.z1);
	t.vertex(bb.x1, bb.y1, bb.z1);
	t.vertex(bb.x0, bb.y1, bb.z1);
	
	t.normal(0.0f, -1.0f, 0.0f);
	t.vertex(bb.x0, bb.y0, bb.z0);
	t.vertex(bb.x1, bb.y0, bb.z0);
	t.vertex(bb.x1, bb.y0, bb.z1);
	t.vertex(bb.x0, bb.y0, bb.z1);
	
	t.normal(0.0f, 1.0f, 0.0f);
	t.vertex(bb.x0, bb.y1, bb.z1);
	t.vertex(bb.x1, bb.y1, bb.z1);
	t.vertex(bb.x1, bb.y1, bb.z0);
	t.vertex(bb.x0, bb.y1, bb.z0);
	
	t.normal(-1.0f, 0.0f, 0.0f);
	t.vertex(bb.x0, bb.y0, bb.z1);
	t.vertex(bb.x0, bb.y1, bb.z1);
	t.vertex(bb.x0, bb.y1, bb.z0);
	t.vertex(bb.x0, bb.y0, bb.z0);
	
	t.normal(1.0f, 0.0f, 0.0f);
	t.vertex(bb.x1, bb.y0, bb.z0);
	t.vertex(bb.x1, bb.y1, bb.z0);
	t.vertex(bb.x1, bb.y1, bb.z1);
	t.vertex(bb.x1, bb.y0, bb.z1);
	t.offset(0.0, 0.0, 0.0);
	t.end();
	glEnable(GL_TEXTURE_2D);
}

void EntityRenderer::renderFlat(AABB &bb)
{
	Tesselator &t = Tesselator::instance;
	t.begin();
	t.vertex(bb.x0, bb.y1, bb.z0);
	t.vertex(bb.x1, bb.y1, bb.z0);
	t.vertex(bb.x1, bb.y0, bb.z0);
	t.vertex(bb.x0, bb.y0, bb.z0);
	t.vertex(bb.x0, bb.y0, bb.z1);
	t.vertex(bb.x1, bb.y0, bb.z1);
	t.vertex(bb.x1, bb.y1, bb.z1);
	t.vertex(bb.x0, bb.y1, bb.z1);
	t.vertex(bb.x0, bb.y0, bb.z0);
	t.vertex(bb.x1, bb.y0, bb.z0);
	t.vertex(bb.x1, bb.y0, bb.z1);
	t.vertex(bb.x0, bb.y0, bb.z1);
	t.vertex(bb.x0, bb.y1, bb.z1);
	t.vertex(bb.x1, bb.y1, bb.z1);
	t.vertex(bb.x1, bb.y1, bb.z0);
	t.vertex(bb.x0, bb.y1, bb.z0);
	t.vertex(bb.x0, bb.y0, bb.z1);
	t.vertex(bb.x0, bb.y1, bb.z1);
	t.vertex(bb.x0, bb.y1, bb.z0);
	t.vertex(bb.x0, bb.y0, bb.z0);
	t.vertex(bb.x1, bb.y0, bb.z0);
	t.vertex(bb.x1, bb.y1, bb.z0);
	t.vertex(bb.x1, bb.y1, bb.z1);
	t.vertex(bb.x1, bb.y0, bb.z1);
	t.end();
}


void EntityRenderer::postRender(Entity &entity, double x, double y, double z, float rot, float a)
{
	if (entityRenderDispatcher.options->fancyGraphics)
	{
		double dist = entityRenderDispatcher.distanceToSqr(entity.x, entity.y, entity.z);
		float pow = static_cast<float>((1.0 - dist / 256.0) * shadowStrength);
		if (pow > 0.0f)
			renderShadow(entity, x, y, z, pow, a);
	}

	if (entity.isOnFire())
		renderFlame(entity, x, y, z, a);
}

Font &EntityRenderer::getFont()
{
	return entityRenderDispatcher.getFont();
}
