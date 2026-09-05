#include "client/renderer/entity/ItemRenderer.h"

#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "client/gui/Font.h"
#include "client/renderer/Tesselator.h"
#include "client/renderer/Textures.h"
#include "world/entity/item/EntityItem.h"
#include "world/item/ItemInstance.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/PistonBaseTile.h"

#include "java/String.h"
#include "util/Mth.h"

#include "OpenGL.h"

#ifndef GL_RESCALE_NORMAL
#define GL_RESCALE_NORMAL 32826
#endif

ItemRenderer::ItemRenderer(EntityRenderDispatcher &entityRenderDispatcher) : EntityRenderer(entityRenderDispatcher), tileRenderer(false, false)
{
	shadowRadius = 0.15f;
	shadowStrength = 0.75f;
}

void ItemRenderer::render(Entity &entity, double x, double y, double z, float rot, float a)
{
	EntityItem &itemEntity = static_cast<EntityItem &>(entity);
	ItemInstance &item = itemEntity.item;

	random.setSeed(187);
	glPushMatrix();
	float bob = Mth::sin((itemEntity.age + a) / 10.0f + itemEntity.bobOffs) * 0.1f + 0.1f;
	float spin = ((itemEntity.age + a) / 20.0f + itemEntity.bobOffs) * (180.0f / Mth::PI);
	int_t count = 1;
	if (item.stackSize > 1) count = 2;
	if (item.stackSize > 5) count = 3;
	if (item.stackSize > 20) count = 4;

	glTranslatef((float)x, (float)(y + bob), (float)z);
	glEnable(GL_RESCALE_NORMAL);

	Tile *tile = item.itemID >= 0 && item.itemID < static_cast<int_t>(Tile::tiles.size()) ? Tile::tiles[item.itemID] : nullptr;
	if (tile != nullptr && TileRenderer::canRender(tile->getRenderShape()))
	{
		glRotatef(spin, 0.0f, 1.0f, 0.0f);
		bindTexture(u"/terrain.png");
		int_t tileColor = tile->getItemColor(item.getAuxValue());
		bool useColorMaterial = tileColor != 0xFFFFFF;
		if (useColorMaterial)
		{
			glEnable(GL_COLOR_MATERIAL);
			glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
			float tr = static_cast<float>((tileColor >> 16) & 255) / 255.0f;
			float tg = static_cast<float>((tileColor >> 8) & 255) / 255.0f;
			float tb = static_cast<float>(tileColor & 255) / 255.0f;
			glColor4f(tr, tg, tb, 1.0f);
		}
		float scale = 0.5f;
		if (tile->isCubeShaped() || tile->id == Tile::pistonBase.id || tile->id == Tile::pistonStickyBase.id)
			scale = 0.25f;
		glScalef(scale, scale, scale);
		for (int_t i = 0; i < count; ++i)
		{
			glPushMatrix();
			if (i > 0)
				glTranslatef((random.nextFloat() * 2.0f - 1.0f) * 0.2f / scale,
					(random.nextFloat() * 2.0f - 1.0f) * 0.2f / scale,
					(random.nextFloat() * 2.0f - 1.0f) * 0.2f / scale);
			tileRenderer.renderTile(*tile, item.getAuxValue());
			glPopMatrix();
		}
		if (useColorMaterial)
			glDisable(GL_COLOR_MATERIAL);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		glScalef(0.5f, 0.5f, 0.5f);
		int_t icon = item.getIcon();
		if (item.itemID < 256)
			bindTexture(u"/terrain.png");
		else
			bindTexture(u"/gui/items.png");
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.1f);
		Tesselator &t = Tesselator::instance;
		float u0 = (icon % 16 * 16 + 0) / 256.0f;
		float u1 = (icon % 16 * 16 + 16) / 256.0f;
		float v0 = (icon / 16 * 16 + 0) / 256.0f;
		float v1 = (icon / 16 * 16 + 16) / 256.0f;
		float xo = 0.5f;
		float yo = 0.25f;
		for (int_t i = 0; i < count; ++i)
		{
			glPushMatrix();
			if (i > 0)
				glTranslatef((random.nextFloat() * 2.0f - 1.0f) * 0.3f,
					(random.nextFloat() * 2.0f - 1.0f) * 0.3f,
					(random.nextFloat() * 2.0f - 1.0f) * 0.3f);
			glRotatef(180.0f - entityRenderDispatcher.playerRotY, 0.0f, 1.0f, 0.0f);
			t.begin();
			t.normal(0.0f, 1.0f, 0.0f);
			t.vertexUV(0.0f - xo, 0.0f - yo, 0.0, u0, v1);
			t.vertexUV(1.0f - xo, 0.0f - yo, 0.0, u1, v1);
			t.vertexUV(1.0f - xo, 1.0f - yo, 0.0, u1, v0);
			t.vertexUV(0.0f - xo, 1.0f - yo, 0.0, u0, v0);
			t.end();
			glPopMatrix();
		}
	}

	glDisable(GL_RESCALE_NORMAL);
	glPopMatrix();
}

void ItemRenderer::renderGuiItem(Font &font, Textures &textures, ItemInstance &item, int_t x, int_t y)
{
	if (item.isEmpty())
		return;

	Tile *tile = item.itemID >= 0 && item.itemID < static_cast<int_t>(Tile::tiles.size()) ? Tile::tiles[item.itemID] : nullptr;
	
	int_t color = tile != nullptr ? tile->getItemColor(item.getAuxValue()) : 0xFFFFFF;
	float r = static_cast<float>((color >> 16) & 0xFF) / 255.0f;
	float g = static_cast<float>((color >> 8) & 0xFF) / 255.0f;
	float b = static_cast<float>(color & 0xFF) / 255.0f;

	if (tile != nullptr && TileRenderer::canRender(tile->getRenderShape()))
	{
		textures.bind(textures.loadTexture(u"/terrain.png"));
		glPushMatrix();
		glTranslatef(static_cast<float>(x - 2), static_cast<float>(y + 3), -3.0f);
		glScalef(10.0f, 10.0f, 10.0f);
		glTranslatef(1.0f, 0.5f, 1.0f);
		glScalef(1.0f, 1.0f, -1.0f);
		glRotatef(210.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
		if (renderWithColor)
			glColor4f(r, g, b, 1.0f);
		glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
		tileRenderer.renderGuiTile(*tile, item.getAuxValue());
		glPopMatrix();
	}
	else if (item.getIcon() >= 0)
	{
		glDisable(GL_LIGHTING);
		if (item.itemID < 256)
			textures.bind(textures.loadTexture(u"/terrain.png"));
		else
			textures.bind(textures.loadTexture(u"/gui/items.png"));
		if (renderWithColor)
			glColor4f(r, g, b, 1.0f);
		blit(x, y, item.getIcon() % 16 * 16, item.getIcon() / 16 * 16, 16, 16);
		glEnable(GL_LIGHTING);
	}

	glEnable(GL_CULL_FACE);
}

void ItemRenderer::renderGuiItemDecorations(Font &font, Textures &textures, ItemInstance &item, int_t x, int_t y)
{
	if (item.isEmpty())
		return;
	if (item.stackSize > 1)
	{
		jstring amount = String::toString(item.stackSize);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		font.drawShadow(amount, x + 17 - font.width(amount), y + 9, 0xFFFFFF);
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
	}
	if (item.isItemDamaged())
	{
		auto fillRect = [](int_t rx, int_t ry, int_t w, int_t h, int_t color) {
			Tesselator &t = Tesselator::instance;
			t.begin();
			t.color(color);
			t.vertex(rx + 0, ry + 0, 0.0);
			t.vertex(rx + 0, ry + h, 0.0);
			t.vertex(rx + w, ry + h, 0.0);
			t.vertex(rx + w, ry + 0, 0.0);
			t.end();
		};

		int_t durabilityWidth = static_cast<int_t>(13.0 - static_cast<double>(item.itemDamage) * 13.0 / static_cast<double>(item.getMaxDamage()) + 0.5);
		int_t durabilityColor = static_cast<int_t>(255.0 - static_cast<double>(item.itemDamage) * 255.0 / static_cast<double>(item.getMaxDamage()) + 0.5);
		int_t barColor = ((255 - durabilityColor) << 16) | (durabilityColor << 8);
		int_t backgroundColor = (((255 - durabilityColor) / 4) << 16) | 16128;
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		fillRect(x + 2, y + 13, 13, 2, 0);
		fillRect(x + 2, y + 13, 12, 1, backgroundColor);
		fillRect(x + 2, y + 13, durabilityWidth, 1, barColor);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void ItemRenderer::blit(int_t x, int_t y, int_t sx, int_t sy, int_t w, int_t h)
{
	float su = 1.0f / 256.0f;
	float sv = 1.0f / 256.0f;
	Tesselator &t = Tesselator::instance;
	t.begin();
	t.vertexUV(x + 0, y + h, 0.0, sx * su, (sy + h) * sv);
	t.vertexUV(x + w, y + h, 0.0, (sx + w) * su, (sy + h) * sv);
	t.vertexUV(x + w, y + 0, 0.0, (sx + w) * su, sy * sv);
	t.vertexUV(x + 0, y + 0, 0.0, sx * su, sy * sv);
	t.end();
}
