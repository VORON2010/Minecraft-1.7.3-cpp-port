#include "client/gui/Gui.h"

#include "client/spc/SPCCommand.h"
#include "client/Lighting.h"
#include "client/gui/ChatScreen.h"
#include "client/gui/ScreenSizeCalculator.h"
#include "client/renderer/Tesselator.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "client/renderer/entity/ItemRenderer.h"

#include "client/Minecraft.h"
#include "world/level/material/LiquidMaterial.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/PortalTile.h"
#include "world/level/tile/PumpkinTile.h"
#include "java/Runtime.h"
#include "java/System.h"
#include "util/Mth.h"
#include <cmath>
#include <cstring>

#ifndef GL_RESCALE_NORMAL
#define GL_RESCALE_NORMAL 32826
#endif

namespace
{
	int_t javaIntAdd(int_t left, int_t right)
	{
		uint_t bits = static_cast<uint_t>(left) + static_cast<uint_t>(right);
		int_t result;
		std::memcpy(&result, &bits, sizeof(result));
		return result;
	}

	int_t javaIntSubtract(int_t left, int_t right)
	{
		uint_t bits = static_cast<uint_t>(left) - static_cast<uint_t>(right);
		int_t result;
		std::memcpy(&result, &bits, sizeof(result));
		return result;
	}

	int_t javaIntMultiply(int_t left, int_t right)
	{
		uint_t bits = static_cast<uint_t>(left) * static_cast<uint_t>(right);
		int_t result;
		std::memcpy(&result, &bits, sizeof(result));
		return result;
	}

	int_t hsbToRgb(float hue, float saturation, float brightness)
	{
		int_t red;
		int_t green;
		int_t blue;
		if (saturation == 0.0f)
		{
			red = green = blue = static_cast<int_t>(brightness * 255.0f + 0.5f);
		}
		else
		{
			float h = static_cast<float>((hue - std::floor(hue)) * 6.0f);
			float f = static_cast<float>(h - std::floor(h));
			float p = brightness * (1.0f - saturation);
			float q = brightness * (1.0f - saturation * f);
			float t = brightness * (1.0f - saturation * (1.0f - f));
			float redFloat;
			float greenFloat;
			float blueFloat;
			switch (static_cast<int_t>(h))
			{
			case 0: redFloat = brightness; greenFloat = t; blueFloat = p; break;
			case 1: redFloat = q; greenFloat = brightness; blueFloat = p; break;
			case 2: redFloat = p; greenFloat = brightness; blueFloat = t; break;
			case 3: redFloat = p; greenFloat = q; blueFloat = brightness; break;
			case 4: redFloat = t; greenFloat = p; blueFloat = brightness; break;
			default: redFloat = brightness; greenFloat = p; blueFloat = q; break;
			}
			red = static_cast<int_t>(redFloat * 255.0f + 0.5f);
			green = static_cast<int_t>(greenFloat * 255.0f + 0.5f);
			blue = static_cast<int_t>(blueFloat * 255.0f + 0.5f);
		}
		return static_cast<int_t>(0xff000000U | (static_cast<uint_t>(red) << 16) |
			(static_cast<uint_t>(green) << 8) | static_cast<uint_t>(blue));
	}
}

Gui::Gui(Minecraft &minecraft) : minecraft(minecraft)
{

}


void Gui::renderPumpkinBlur(int_t width, int_t height)
{
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_ALPHA_TEST);
	glBindTexture(GL_TEXTURE_2D, minecraft.textures.loadTexture(u"%blur%/misc/pumpkinblur.png"));
	Tesselator &t = Tesselator::instance;
	t.begin();
	t.vertexUV(0.0, height, -90.0, 0.0, 1.0);
	t.vertexUV(width, height, -90.0, 1.0, 1.0);
	t.vertexUV(width, 0.0, -90.0, 1.0, 0.0);
	t.vertexUV(0.0, 0.0, -90.0, 0.0, 0.0);
	t.end();
	glDepthMask(true);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}


void Gui::renderVignette(float brightness, int_t width, int_t height)
{
	brightness = 1.0f - brightness;
	if (brightness < 0.0f)
		brightness = 0.0f;
	if (brightness > 1.0f)
		brightness = 1.0f;

	prevVignetteBrightness = static_cast<float>(prevVignetteBrightness + (brightness - prevVignetteBrightness) * 0.01);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
	glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
	glColor4f(prevVignetteBrightness, prevVignetteBrightness, prevVignetteBrightness, 1.0f);
	glBindTexture(GL_TEXTURE_2D, minecraft.textures.loadTexture(u"%blur%/misc/vignette.png"));
	Tesselator &t = Tesselator::instance;
	t.begin();
	t.vertexUV(0.0, height, -90.0, 0.0, 1.0);
	t.vertexUV(width, height, -90.0, 1.0, 1.0);
	t.vertexUV(width, 0.0, -90.0, 1.0, 0.0);
	t.vertexUV(0.0, 0.0, -90.0, 0.0, 0.0);
	t.end();
	glDepthMask(true);
	glEnable(GL_DEPTH_TEST);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void Gui::renderPortalOverlay(float portalTime, int_t width, int_t height)
{
	if (portalTime < 1.0f)
	{
		portalTime *= portalTime;
		portalTime *= portalTime;
		portalTime = portalTime * 0.8f + 0.2f;
	}

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0f, 1.0f, 1.0f, portalTime);
	glBindTexture(GL_TEXTURE_2D, minecraft.textures.loadTexture(u"/terrain.png"));
	float u0 = Tile::portal.tex % 16 / 16.0f;
	float v0 = Tile::portal.tex / 16 / 16.0f;
	float u1 = (Tile::portal.tex % 16 + 1) / 16.0f;
	float v1 = (Tile::portal.tex / 16 + 1) / 16.0f;
	Tesselator &t = Tesselator::instance;
	t.begin();
	t.vertexUV(0.0, height, -90.0, u0, v1);
	t.vertexUV(width, height, -90.0, u1, v1);
	t.vertexUV(width, 0.0, -90.0, u1, v0);
	t.vertexUV(0.0, 0.0, -90.0, u0, v0);
	t.end();
	glDepthMask(true);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void Gui::render(float a, bool inScreen, int_t xm, int_t ym)
{
	ScreenSizeCalculator ssc(minecraft.options, minecraft.width, minecraft.height);
	int_t width = ssc.getWidth();
	int_t height = ssc.getHeight();

	Font &font = *minecraft.font;

	minecraft.gameRenderer.setupGuiScreen();

	glEnable(GL_BLEND);

	if (minecraft.options.fancyGraphics)
		renderVignette(minecraft.player->getBrightness(a), width, height);

	ItemInstance &helmet = minecraft.player->inventory.armorInventory[3];
	if (!minecraft.options.thirdPersonView && !helmet.isEmpty() && helmet.itemID == Tile::pumpkin.id)
		renderPumpkinBlur(width, height);

	float portalTime = minecraft.player->oPortalTime + (minecraft.player->portalTime - minecraft.player->oPortalTime) * a;
	if (portalTime > 0.0f)
		renderPortalOverlay(portalTime, width, height);

	
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, minecraft.textures.loadTexture(u"/gui/gui.png"));

	blit(width / 2 - 91, height - 22, 0, 0, 182, 22);
	blit(width / 2 - 91 - 1 + minecraft.player->inventory.currentItem * 20, height - 22 - 1, 0, 22, 24, 22);

	
	glBindTexture(GL_TEXTURE_2D, minecraft.textures.loadTexture(u"/gui/icons.png"));
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
	blit(width / 2 - 7, height / 2 - 7, 0, 0, 16, 16);
	glDisable(GL_BLEND);

	
	bool flicker = (minecraft.player->invulnerableTime / 3) % 2 == 1;
	if (minecraft.player->invulnerableTime < 10)
		flicker = false;

	int_t nowHealth = minecraft.player->health;
	int_t lastHealth = minecraft.player->lastHealth;

	random.setSeed(javaIntMultiply(tickCount, 312871));

	if (minecraft.gameMode->canHurtPlayer())
	{
		int_t armor = minecraft.player->inventory.getArmorValue();

		for (int_t x = 0; x < 10; x++)
		{
			int_t y = height - 32;
			if (armor > 0)
			{
				int_t armorX = width / 2 + 91 - x * 8 - 9;
				if (x * 2 + 1 < armor) blit(armorX, y, 34, 9, 9, 9);
				if (x * 2 + 1 == armor) blit(armorX, y, 25, 9, 9, 9);
				if (x * 2 + 1 > armor) blit(armorX, y, 16, 9, 9, 9);
			}

			int_t healthX = width / 2 - 91 + x * 8;

			if (nowHealth <= 4)
				y += random.nextInt(2);

			blit(healthX, y, 16 + flicker * 9, 0, 9, 9);
			if (flicker)
			{
				if (x * 2 + 1 < lastHealth) blit(healthX, y, 70, 0, 9, 9);
				if (x * 2 + 1 == lastHealth) blit(healthX, y, 79, 0, 9, 9);
			}
			if (x * 2 + 1 < nowHealth) blit(healthX, y, 52, 0, 9, 9);
			if (x * 2 + 1 == nowHealth) blit(healthX, y, 61, 0, 9, 9);
		}

		if (minecraft.player->isUnderLiquid(Material::water))
		{
			int_t full = static_cast<int_t>(std::ceil(static_cast<double>(minecraft.player->airSupply - 2) * 10.0 / 300.0));
			int_t partial = static_cast<int_t>(std::ceil(static_cast<double>(minecraft.player->airSupply) * 10.0 / 300.0)) - full;

			for (int_t i = 0; i < full + partial; i++)
			{
				if (i < full)
					blit(width / 2 - 91 + i * 8, height - 32 - 9, 16, 18, 9, 9);
				else
					blit(width / 2 - 91 + i * 8, height - 32 - 9, 25, 18, 9, 9);
			}
		}
	}

	glDisable(GL_BLEND);
	glEnable(GL_RESCALE_NORMAL);
	glPushMatrix();
	glRotatef(120.0f, 1.0f, 0.0f, 0.0f);
	Lighting::turnOn();
	glPopMatrix();
	for (int_t i = 0; i < 9; i++)
	{
		int_t x = width / 2 - 90 + i * 20 + 2;
		int_t y = height - 16 - 3;
		renderSlot(i, x, y, a);
	}
	Lighting::turnOff();
	glDisable(GL_RESCALE_NORMAL);

	
	if (minecraft.player->sleepTimer > 0)
	{
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		int_t timer = minecraft.player->sleepTimer;
		constexpr int_t SLEEP_DURATION = 100;
		constexpr int_t WAKE_UP_DURATION = 10;
		float amount = static_cast<float>(timer) / static_cast<float>(SLEEP_DURATION);
		if (amount > 1.0f)
		{
			amount = 1.0f - (static_cast<float>(timer - SLEEP_DURATION) / static_cast<float>(WAKE_UP_DURATION));
		}

		int_t alpha = static_cast<int_t>(220.0f * amount);
		if (alpha < 0) alpha = 0;
		if (alpha > 220) alpha = 220;
		int_t color = (alpha << 24) | 0x101020;
		fill(0, 0, width, height, color);

		glEnable(GL_ALPHA_TEST);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
	}

	
	if (minecraft.options.showDebugInfo)
	{
		font.drawShadow(Minecraft::VERSION_STRING + u" (" + minecraft.fpsString + u")", 2, 2, 0xFFFFFF);
		font.drawShadow(minecraft.gatherStats1(), 2, 12, 0xFFFFFF);
		font.drawShadow(minecraft.gatherStats2(), 2, 22, 0xFFFFFF);
		font.drawShadow(minecraft.gatherStats3(), 2, 32, 0xFFFFFF);
		font.drawShadow(minecraft.gatherStats4(), 2, 42, 0xFFFFFF);

		long_t maxMemory = Runtime::getRuntime().maxMemory();
		long_t totalMemory = Runtime::getRuntime().totalMemory();
		long_t freeMemory = Runtime::getRuntime().freeMemory();
		long_t usedMemory = totalMemory - freeMemory;

		jstring str = u"Used memory: " + String::toString(usedMemory * 100 / maxMemory) + u"% (" + String::toString(usedMemory / 1024 / 1024) + u"MB) of " + String::toString(maxMemory / 1024 / 1024) + u"MB";
		drawString(font, str, width - font.width(str) - 2, 2, 0xE0E0E0);
		str = u"Allocated memory: " + String::toString(totalMemory * 100 / maxMemory) + u"% (" + String::toString(totalMemory / 1024 / 1024) + u"MB)";
		drawString(font, str, width - font.width(str) - 2, 12, 0xE0E0E0);

		drawString(font, u"x: " + String::toString(minecraft.player->x), 2, 64, 0xE0E0E0);
		drawString(font, u"y: " + String::toString(minecraft.player->y), 2, 72, 0xE0E0E0);
		drawString(font, u"z: " + String::toString(minecraft.player->z), 2, 80, 0xE0E0E0);
		int_t facing = Mth::floor(static_cast<double>(minecraft.player->yRot * 4.0f / 360.0f) + 0.5) & 3;
		drawString(font, u"f: " + String::toString(facing), 2, 88, 0xE0E0E0);
	}

	if (nowPlayingTime > 0)
	{
		float remaining = nowPlayingTime - a;
		int_t alpha = static_cast<int_t>(remaining * 256.0f / 20.0f);
		if (alpha > 255)
			alpha = 255;
		if (alpha > 0)
		{
			glPushMatrix();
			glTranslatef(static_cast<float>(width / 2), static_cast<float>(height - 48), 0.0f);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			int_t color = 0xffffff;
			if (animateNowPlayingColor)
				color = hsbToRgb(remaining / 50.0f, 0.7f, 0.6f) & 0xffffff;
			font.draw(nowPlayingString, -font.width(nowPlayingString) / 2, -4,
				static_cast<int_t>(static_cast<uint_t>(color) | (static_cast<uint_t>(alpha) << 24)));
			glDisable(GL_BLEND);
			glPopMatrix();
		}
	}

	
	{
		bool chatOpen = dynamic_cast<ChatScreen*>(minecraft.screen.get()) != nullptr;
		int_t maxVisible = chatOpen ? 20 : 10;

		struct VisibleLine { jstring text; int_t alpha; };
		std::vector<VisibleLine> visibleLines;

		for (auto it = SPCCommand::messages.rbegin(); it != SPCCommand::messages.rend(); ++it)
		{
			int_t age = javaIntSubtract(tickCount, it->tickCreated);
			if (!chatOpen && age > SPCCommand::MESSAGE_DISPLAY_TICKS)
				break;

			int_t alpha;
			if (chatOpen)
			{
				alpha = 255;
			}
			else
			{
				double fade = 1.0 - static_cast<double>(age) / static_cast<double>(SPCCommand::MESSAGE_DISPLAY_TICKS);
				fade *= 10.0;
				if (fade < 0.0) fade = 0.0;
				if (fade > 1.0) fade = 1.0;
				fade *= fade;
				alpha = static_cast<int_t>(255.0 * fade);
			}

			if (alpha <= 0)
				continue;

			visibleLines.push_back({it->text, alpha});

			if (static_cast<int_t>(visibleLines.size()) >= maxVisible)
				break;
		}

		if (!visibleLines.empty())
		{
			int_t msgY = height - 48;
			int_t drawn = 0;

		glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_ALPHA_TEST);

			for (auto &line : visibleLines)
			{
				if (drawn >= maxVisible)
					break;

				fill(2, msgY - 1, 322, msgY + 8, (line.alpha / 2) << 24);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				font.drawShadow(line.text, 2, msgY, 0xFFFFFF | (line.alpha << 24));
				msgY -= 9;
				drawn++;
			}

			glEnable(GL_ALPHA_TEST);
			glDisable(GL_BLEND);
		}
	}
}

void Gui::renderSlot(int_t slot, int_t x, int_t y, float a)
{
	static ItemRenderer itemRenderer(EntityRenderDispatcher::instance);
	ItemInstance &stack = minecraft.player->inventory.mainInventory[slot];
	if (stack.isEmpty())
		return;

	float pop = static_cast<float>(stack.popTime) - a;
	if (pop > 0.0f)
	{
		glPushMatrix();
		float scale = 1.0f + pop / 5.0f;
		glTranslatef(static_cast<float>(x + 8), static_cast<float>(y + 12), 0.0f);
		glScalef(1.0f / scale, (scale + 1.0f) / 2.0f, 1.0f);
		glTranslatef(static_cast<float>(-(x + 8)), static_cast<float>(-(y + 12)), 0.0f);
	}

	itemRenderer.renderGuiItem(*minecraft.font, minecraft.textures, stack, x, y);

	if (pop > 0.0f)
		glPopMatrix();

	itemRenderer.renderGuiItemDecorations(*minecraft.font, minecraft.textures, stack, x, y);
}


void Gui::tick()
{
	if (nowPlayingTime > 0) nowPlayingTime--;

	tickCount = javaIntAdd(tickCount, 1);
	SPCCommand::guiTickCount = tickCount;
}

void Gui::setRecordPlayingMessage(const jstring &name)
{
	nowPlayingString = u"Now playing: " + name;
	nowPlayingTime = 60;
	animateNowPlayingColor = true;
}
