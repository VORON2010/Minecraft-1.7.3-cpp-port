#include "client/renderer/ItemInHandRenderer.h"

#include "client/Minecraft.h"
#include "client/Lighting.h"
#include "client/gui/Font.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "client/renderer/Textures.h"
#include "client/renderer/entity/PlayerRenderer.h"
#include "client/renderer/Tesselator.h"
#include "client/renderer/texturefx/TileSize.h"

#include "util/Mth.h"

#include "world/item/Item.h"
#include "world/item/ItemMap.h"
#include "world/item/Items.h"
#include "world/level/MapData.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/level/tile/FireTile.h"

ItemInHandRenderer::ItemInHandRenderer(Minecraft &mc) : mc(mc)
{

}

void ItemInHandRenderer::renderMapFirstPerson(float a, float h)
{
	auto &localPlayer = *mc.player;

	if (mc.font == nullptr)
		return;
	if (mapItemRenderer == nullptr)
		mapItemRenderer = std::make_unique<MapItemRenderer>(*mc.font, mc.options);

	float swing = localPlayer.getAttackAnim(a);
	float swing1 = Mth::sin(swing * Mth::PI);
	float swing2 = Mth::sin(Mth::sqrt(swing) * Mth::PI);
	glTranslatef(-swing2 * 0.4f, Mth::sin(Mth::sqrt(swing) * Mth::PI * 2.0f) * 0.2f, -swing1 * 0.2f);

	float pitch = 1.0f - (localPlayer.xRotO + (localPlayer.xRot - localPlayer.xRotO) * a) / 45.0f + 0.1f;
	if (pitch < 0.0f) pitch = 0.0f;
	if (pitch > 1.0f) pitch = 1.0f;
	pitch = -Mth::cos(pitch * Mth::PI) * 0.5f + 0.5f;

	float d = 0.8f;
	glTranslatef(0.0f, -(1.0f - h) * 1.2f - pitch * 0.5f + 0.04f, -0.9f * d);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(pitch * -85.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_RESCALE_NORMAL);

	
	{
		const jstring fallbackTexture = localPlayer.getTexture();
		glBindTexture(GL_TEXTURE_2D, mc.textures.loadHttpTexture(localPlayer.customTextureUrl, &fallbackTexture));
		auto &playerRenderer = EntityRenderDispatcher::playerRenderer;
		for (int i = 0; i < 2; i++)
		{
			int flip = i * 2 - 1;
			glPushMatrix();
			glTranslatef(0.0f, -0.6f, 1.1f * flip);
			glRotatef(-45.0f * flip, 1.0f, 0.0f, 0.0f);
			glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
			glRotatef(59.0f, 0.0f, 0.0f, 1.0f);
			glRotatef(-65.0f * flip, 0.0f, 1.0f, 0.0f);
			playerRenderer.renderHand();
			glPopMatrix();
		}
	}

	swing = localPlayer.getAttackAnim(a);
	swing1 = Mth::sin(swing * swing * Mth::PI);
	swing2 = Mth::sin(Mth::sqrt(swing) * Mth::PI);
	glRotatef(-swing1 * 20.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(-swing2 * 20.0f, 0.0f, 0.0f, 1.0f);
	glRotatef(-swing2 * 80.0f, 1.0f, 0.0f, 0.0f);

	float scale = 0.38f;
	glScalef(scale, scale, scale);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
	glTranslatef(-1.0f, -1.0f, 0.0f);

	float mapScale = 2.0f / 128.0f;
	glScalef(mapScale, mapScale, mapScale);

	
	Tesselator &t = Tesselator::instance;
	glBindTexture(GL_TEXTURE_2D, mc.textures.loadTexture(u"/misc/mapbg.png"));
	glNormal3f(0.0f, 0.0f, -1.0f);
	constexpr int_t bgBorder = 7;
	t.begin();
	t.vertexUV((float)(0 - bgBorder), (float)(128 + bgBorder), 0.0f, 0.0f, 1.0f);
	t.vertexUV((float)(128 + bgBorder), (float)(128 + bgBorder), 0.0f, 1.0f, 1.0f);
	t.vertexUV((float)(128 + bgBorder), (float)(0 - bgBorder), 0.0f, 1.0f, 0.0f);
	t.vertexUV((float)(0 - bgBorder), (float)(0 - bgBorder), 0.0f, 0.0f, 0.0f);
	t.end();

	
	ItemInstance item = selectedItem;
	if (!item.isEmpty() && Items::map != nullptr && item.itemID == Items::map->getShiftedIndex())
	{
		MapData *data = ItemMap::getMapData(static_cast<short_t>(item.itemDamage), *mc.level);
		if (data != nullptr)
			mapItemRenderer->render(*data, mc.textures);
	}
}

namespace HeldItemRenderer
{
	void render(Textures &textures, TileRenderer &tileRenderer, ItemInstance &item, float brightness)
	{
		Tile *tile = item.itemID >= 0 && item.itemID < static_cast<int_t>(Tile::tiles.size()) ? Tile::tiles[item.itemID] : nullptr;
		bool renderedAsBlock = false;
		if (tile != nullptr && TileRenderer::canRender(tile->getRenderShape()))
		{
			glBindTexture(GL_TEXTURE_2D, textures.loadTexture(u"/terrain.png"));
			int_t tileColor = tile->getItemColor(item.getAuxValue());
			glEnable(GL_COLOR_MATERIAL);
			glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
			float tr = static_cast<float>((tileColor >> 16) & 255) / 255.0f;
			float tg = static_cast<float>((tileColor >> 8) & 255) / 255.0f;
			float tb = static_cast<float>(tileColor & 255) / 255.0f;
			glColor4f(tr * brightness, tg * brightness, tb * brightness, 1.0f);
			tileRenderer.renderTile(*tile, item.getAuxValue());
			glDisable(GL_COLOR_MATERIAL);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			renderedAsBlock = true;
		}
	
		if (!renderedAsBlock && !item.isEmpty())
		{
			if (item.itemID < 256)
				glBindTexture(GL_TEXTURE_2D, textures.loadTexture(u"/terrain.png"));
			else
				glBindTexture(GL_TEXTURE_2D, textures.loadTexture(u"/gui/items.png"));
	
			Tesselator &t = Tesselator::instance;
			int_t icon = item.getIcon();
			float u1 = (icon % 16 * TileSize::size + 0.0f) / TileSize::size16;
			float u0 = (icon % 16 * TileSize::size + TileSize::sizeMinus0_01) / TileSize::size16;
			float v0 = (icon / 16 * TileSize::size + 0.0f) / TileSize::size16;
			float v1 = (icon / 16 * TileSize::size + TileSize::sizeMinus0_01) / TileSize::size16;
			float r = 1.0f;
			float xo = 0.0f;
			float yo = 0.3f;
			glEnable(GL_RESCALE_NORMAL);
			glTranslatef(-xo, -yo, 0.0f);
			float s = 1.5f;
			glScalef(s, s, s);
			glRotatef(50.0f, 0.0f, 1.0f, 0.0f);
			glRotatef(335.0f, 0.0f, 0.0f, 1.0f);
			glTranslatef(-0.9375f, -0.0625f, 0.0f);
			float dd = 0.0625f;
	
			t.begin();
			t.normal(0.0f, 0.0f, 1.0f);
			t.vertexUV(0.0, 0.0, 0.0, u0, v1);
			t.vertexUV(r, 0.0, 0.0, u1, v1);
			t.vertexUV(r, 1.0, 0.0, u1, v0);
			t.vertexUV(0.0, 1.0, 0.0, u0, v0);
			t.end();
	
			t.begin();
			t.normal(0.0f, 0.0f, -1.0f);
			t.vertexUV(0.0, 1.0, 0.0f - dd, u0, v0);
			t.vertexUV(r, 1.0, 0.0f - dd, u1, v0);
			t.vertexUV(r, 0.0, 0.0f - dd, u1, v1);
			t.vertexUV(0.0, 0.0, 0.0f - dd, u0, v1);
			t.end();
	
			t.begin();
			t.normal(-1.0f, 0.0f, 0.0f);
			for (int_t i = 0; i < TileSize::size; i++)
			{
				float p = i / TileSize::sizeFloat;
				float uu = u0 + (u1 - u0) * p - TileSize::texNudge;
				float xx = r * p;
				t.vertexUV(xx, 0.0, 0.0f - dd, uu, v1);
				t.vertexUV(xx, 0.0, 0.0, uu, v1);
				t.vertexUV(xx, 1.0, 0.0, uu, v0);
				t.vertexUV(xx, 1.0, 0.0f - dd, uu, v0);
			}
			t.end();
	
			t.begin();
			t.normal(1.0f, 0.0f, 0.0f);
			for (int_t i = 0; i < TileSize::size; i++)
			{
				float p = i / TileSize::sizeFloat;
				float uu = u0 + (u1 - u0) * p - TileSize::texNudge;
				float xx = r * p + TileSize::reciprocal;
				t.vertexUV(xx, 1.0, 0.0f - dd, uu, v0);
				t.vertexUV(xx, 1.0, 0.0, uu, v0);
				t.vertexUV(xx, 0.0, 0.0, uu, v1);
				t.vertexUV(xx, 0.0, 0.0f - dd, uu, v1);
			}
			t.end();
	
			t.begin();
			t.normal(0.0f, 1.0f, 0.0f);
			for (int_t i = 0; i < TileSize::size; i++)
			{
				float p = i / TileSize::sizeFloat;
				float vv = v1 + (v0 - v1) * p - TileSize::texNudge;
				float yy = r * p + TileSize::reciprocal;
				t.vertexUV(0.0, yy, 0.0, u0, vv);
				t.vertexUV(r, yy, 0.0, u1, vv);
				t.vertexUV(r, yy, 0.0f - dd, u1, vv);
				t.vertexUV(0.0, yy, 0.0f - dd, u0, vv);
			}
			t.end();
	
			t.begin();
			t.normal(0.0f, -1.0f, 0.0f);
			for (int_t i = 0; i < TileSize::size; i++)
			{
				float p = i / TileSize::sizeFloat;
				float vv = v1 + (v0 - v1) * p - TileSize::texNudge;
				float yy = r * p;
				t.vertexUV(r, yy, 0.0, u1, vv);
				t.vertexUV(0.0, yy, 0.0, u0, vv);
				t.vertexUV(0.0, yy, 0.0f - dd, u0, vv);
				t.vertexUV(r, yy, 0.0f - dd, u1, vv);
			}
			t.end();
			glDisable(GL_RESCALE_NORMAL);
		}
	}
}

void ItemInHandRenderer::renderItem(ItemInstance &item, float brightness)
{
	glPushMatrix();
	HeldItemRenderer::render(mc.textures, tileRenderer, item, brightness);
	glPopMatrix();
}

void ItemInHandRenderer::render(float a)
{
	float h = oHeight + (height - oHeight) * a;
	auto &localPlayer = *mc.player;

	glPushMatrix();
	glRotatef(localPlayer.xRotO + (localPlayer.xRot - localPlayer.xRotO) * a, 1.0f, 0.0f, 0.0f);
	glRotatef(localPlayer.yRotO + (localPlayer.yRot - localPlayer.yRotO) * a, 0.0f, 1.0f, 0.0f);
	Lighting::turnOn();
	glPopMatrix();

	float br = mc.level->getBrightness(Mth::floor(localPlayer.x), Mth::floor(localPlayer.y), Mth::floor(localPlayer.z));
	glColor4f(br, br, br, 1.0f);

	ItemInstance item = selectedItem;
	if (!item.isEmpty())
	{
		glPushMatrix();
		if (Items::map != nullptr && item.itemID == Items::map->getShiftedIndex())
		{
			renderMapFirstPerson(a, h);
		}
		else
		{
			float d = 0.8f;
			float swing = localPlayer.getAttackAnim(a);
			float swing1 = Mth::sin(swing * Mth::PI);
			float swing2 = Mth::sin(Mth::sqrt(swing) * Mth::PI);
			glTranslatef(-swing2 * 0.4f, Mth::sin(Mth::sqrt(swing) * Mth::PI * 2.0f) * 0.2f, -swing1 * 0.2f);
			glTranslatef(0.7f * d, -0.65f * d - (1.0f - h) * 0.6f, -0.9f * d);
			glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
			glEnable(GL_RESCALE_NORMAL);
			swing = localPlayer.getAttackAnim(a);
			swing1 = Mth::sin(swing * swing * Mth::PI);
			swing2 = Mth::sin(Mth::sqrt(swing) * Mth::PI);
			glRotatef(-swing1 * 20.0f, 0.0f, 1.0f, 0.0f);
			glRotatef(-swing2 * 20.0f, 0.0f, 0.0f, 1.0f);
			glRotatef(-swing2 * 80.0f, 1.0f, 0.0f, 0.0f);
			float scale = 0.4f;
			glScalef(scale, scale, scale);
			Item *itemType = item.getItem();
			if (itemType != nullptr && itemType->shouldRotateAroundWhenRendering())
				glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			renderItem(item, localPlayer.getBrightness(1.0f));
			glDisable(GL_RESCALE_NORMAL);
		}
		glPopMatrix();
	}
	else
	{
		glPushMatrix();
		float d = 0.8f;
		float swing = localPlayer.getAttackAnim(a);
		float swing1 = Mth::sin(swing * Mth::PI);
		float swing2 = Mth::sin(Mth::sqrt(swing) * Mth::PI);
		glTranslatef(-swing2 * 0.3f, Mth::sin(Mth::sqrt(swing) * Mth::PI * 2.0f) * 0.4f, -swing1 * 0.4f);
		glTranslatef(0.8f * d, -0.75f * d - (1.0f - h) * 0.6f, -0.9f * d);
		glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
		glEnable(GL_RESCALE_NORMAL);
		swing = localPlayer.getAttackAnim(a);
		float swing3 = Mth::sin(swing * swing * Mth::PI);
		swing2 = Mth::sin(Mth::sqrt(swing) * Mth::PI);
		glRotatef(swing2 * 70.0f, 0.0f, 1.0f, 0.0f);
		glRotatef(-swing3 * 20.0f, 0.0f, 0.0f, 1.0f);
		const jstring fallbackTexture = localPlayer.getTexture();
		glBindTexture(GL_TEXTURE_2D, mc.textures.loadHttpTexture(localPlayer.customTextureUrl, &fallbackTexture));
		glTranslatef(-1.0f, 3.6f, 3.5f);
		glRotatef(120.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(200.0f, 1.0f, 0.0f, 0.0f);
		glRotatef(-135.0f, 0.0f, 1.0f, 0.0f);
		glScalef(1.0f, 1.0f, 1.0f);
		glTranslatef(5.6f, 0.0f, 0.0f);
		auto &playerRenderer = EntityRenderDispatcher::playerRenderer;
		float ss = 1.0f;
		glScalef(ss, ss, ss);
		playerRenderer.renderHand();
		glPopMatrix();
	}

	glDisable(GL_RESCALE_NORMAL);
	Lighting::turnOff();
}


void ItemInHandRenderer::renderScreenEffect(float a)
{
	glDisable(GL_ALPHA_TEST);

	if (mc.player->isOnFire())
	{
		int_t id = mc.textures.loadTexture(u"/terrain.png");
		glBindTexture(GL_TEXTURE_2D, id);
		renderFire(a);
	}

	if (mc.player->isInWall())
	{
		int_t x = Mth::floor(mc.player->x);
		int_t y = Mth::floor(mc.player->y);
		int_t z = Mth::floor(mc.player->z);

		int_t id = mc.textures.loadTexture(u"/terrain.png");
		glBindTexture(GL_TEXTURE_2D, id);
		int_t tile = mc.level->getTile(x, y, z);
		if (mc.level->isBlockNormalCube(x, y, z))
		{
			renderTex(a, Tile::tiles[tile]->getTexture(Facing::NORTH));
		}
		else
		{
			for (int_t i = 0; i < 8; i++)
			{
				float xo = ((i >> 0) % 2 - 0.5f) * mc.player->bbWidth * 0.9f;
				float yo = ((i >> 1) % 2 - 0.5f) * mc.player->bbHeight * 0.2f;
				float zo = ((i >> 2) % 2 - 0.5f) * mc.player->bbWidth * 0.9f;
				int_t tx = Mth::floor(static_cast<float>(x) + xo);
				int_t ty = Mth::floor(static_cast<float>(y) + yo);
				int_t tz = Mth::floor(static_cast<float>(z) + zo);
				if (mc.level->isBlockNormalCube(tx, ty, tz))
					tile = mc.level->getTile(tx, ty, tz);
			}
		}

		if (Tile::tiles[tile] != nullptr)
			renderTex(a, Tile::tiles[tile]->getTexture(Facing::NORTH));
	}

	if (mc.player->isUnderLiquid(Material::water))
	{
		int_t id = mc.textures.loadTexture(u"/misc/water.png");
		glBindTexture(GL_TEXTURE_2D, id);
		renderWater(a);
	}
	glEnable(GL_ALPHA_TEST);
}


void ItemInHandRenderer::renderTex(float a, int_t tex)
{
	Tesselator &t = Tesselator::instance;
	float brightness = mc.player->getBrightness(a);
	brightness = 0.1f;
	glColor4f(brightness, brightness, brightness, 0.5f);
	glPushMatrix();
	float u0 = tex % 16 / 256.0f - 0.0078125f;
	float u1 = (tex % 16 + 15.99f) / 256.0f + 0.0078125f;
	float v0 = tex / 16 / 256.0f - 0.0078125f;
	float v1 = (tex / 16 + 15.99f) / 256.0f + 0.0078125f;
	t.begin();
	t.vertexUV(-1.0, -1.0, -0.5, u1, v1);
	t.vertexUV(1.0, -1.0, -0.5, u0, v1);
	t.vertexUV(1.0, 1.0, -0.5, u0, v0);
	t.vertexUV(-1.0, 1.0, -0.5, u1, v0);
	t.end();
	glPopMatrix();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}


void ItemInHandRenderer::renderWater(float a)
{
	Tesselator &t = Tesselator::instance;

	float br = mc.player->getBrightness(a);
	glColor4f(br, br, br, 0.5f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPushMatrix();

	float uo = -mc.player->yRot / 64.0f;
	float vo = mc.player->xRot / 64.0f;

	t.begin();
	t.vertexUV(-1.0, -1.0, -0.5, 4.0 + uo, 4.0 + vo);
	t.vertexUV( 1.0, -1.0, -0.5, 0.0 + uo, 4.0 + vo);
	t.vertexUV( 1.0,  1.0, -0.5, 0.0 + uo, 0.0 + vo);
	t.vertexUV(-1.0,  1.0, -0.5, 4.0 + uo, 0.0 + vo);
	t.end();

	glPopMatrix();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);
}


void ItemInHandRenderer::renderFire(float a)
{
	Tesselator &t = Tesselator::instance;
	glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float size = 1.0f;

	for (int_t i = 0; i < 2; i++)
	{
		glPushMatrix();
		int_t tex = Tile::fire.tex + i * 16;
		int_t tx = (tex & 15) << 4;
		int_t ty = tex & 240;
		float u0 = tx / 256.0f;
		float u1 = (tx + 15.99f) / 256.0f;
		float v0 = ty / 256.0f;
		float v1 = (ty + 15.99f) / 256.0f;
		float x0 = (0.0f - size) / 2.0f;
		float x1 = x0 + size;
		float y0 = 0.0f - size / 2.0f;
		float y1 = y0 + size;
		float z = -0.5f;
		glTranslatef(-(i * 2 - 1) * 0.24f, -0.3f, 0.0f);
		glRotatef((i * 2 - 1) * 10.0f, 0.0f, 1.0f, 0.0f);
		t.begin();
		t.vertexUV(x0, y0, z, u1, v1);
		t.vertexUV(x1, y0, z, u0, v1);
		t.vertexUV(x1, y1, z, u0, v0);
		t.vertexUV(x0, y1, z, u1, v0);
		t.end();
		glPopMatrix();
	}

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);
}

void ItemInHandRenderer::tick()
{
	oHeight = height;

	auto &localPlayer = *mc.player;
	ItemInstance *selected = localPlayer.inventory.getSelected();
	std::shared_ptr<const ItemInstanceReference> selectedReference =
		selected == nullptr ? nullptr : selected->retainReference();
	bool selectedEmpty = selected == nullptr;
	bool selectedItemEmpty = selectedItem.isEmpty();
	bool matches = lastSlot == localPlayer.inventory.currentItem &&
		selectedReference == selectedItemReference;

	if (selectedEmpty && selectedItemEmpty)
	{
		matches = true;
	}
	else if (!selectedEmpty && !selectedItemEmpty)
	{
		if (selectedReference == selectedItemReference)
		{
			selectedItem = *selected;
		}
		else if (selected->itemID.load(std::memory_order_relaxed) ==
			selectedItem.itemID.load(std::memory_order_relaxed) &&
			selected->itemDamage.load(std::memory_order_relaxed) ==
			selectedItem.itemDamage.load(std::memory_order_relaxed))
		{
			selectedItem = *selected;
			selectedItemReference = selectedReference;
			matches = true;
		}
	}
	else
	{
		matches = false;
	}

	float max = 0.4f;
	float tHeight = matches ? 1.0f : 0.0f;
	float dd = tHeight - height;
	if (dd < -max) dd = -max;
	if (dd > max) dd = max;
	height += dd;
	if (height < 0.1f)
	{
		if (selected != nullptr)
		{
			selectedItem = *selected;
			selectedItemReference = selectedReference;
		}
		else
		{
			selectedItem = ItemInstance();
			selectedItemReference.reset();
		}
		lastSlot = localPlayer.inventory.currentItem;
	}
}

void ItemInHandRenderer::itemPlaced()
{
	height = 0.0f;
}

void ItemInHandRenderer::itemUsed()
{
	height = 0.0f;
}
