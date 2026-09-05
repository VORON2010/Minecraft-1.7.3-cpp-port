#include "client/gui/AchievementsScreen.h"

#include <algorithm>
#include <cmath>

#include "OpenGL.h"
#include "client/Lighting.h"
#include "client/Minecraft.h"
#include "client/gui/SmallButton.h"
#include "client/renderer/Textures.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "client/renderer/entity/ItemRenderer.h"
#include "java/Random.h"
#include "java/System.h"
#include "lwjgl/Mouse.h"
#include "util/Mth.h"
#include "world/level/tile/DirtTile.h"
#include "world/level/tile/OreTile.h"
#include "world/level/tile/RedstoneOreTile.h"
#include "world/level/tile/SandTile.h"
#include "world/level/tile/StoneTile.h"
#include "world/level/tile/Tile.h"
#include "world/stats/Achievement.h"
#include "world/stats/AchievementList.h"
#include "world/stats/StatCollector.h"
#include "world/stats/StatFileWriter.h"

#ifndef GL_RESCALE_NORMAL
#define GL_RESCALE_NORMAL 32826
#endif

namespace
{
	const int_t PANEL_WIDTH = 256;
	const int_t PANEL_HEIGHT = 202;
	const int_t MAP_WIDTH = 224;
	const int_t MAP_HEIGHT = 155;

	int_t minimumScrollX()
	{
		return AchievementList::minDisplayColumn * 24 - 112;
	}

	int_t minimumScrollY()
	{
		return AchievementList::minDisplayRow * 24 - 112;
	}

	int_t maximumScrollX()
	{
		return AchievementList::maxDisplayColumn * 24 - 77;
	}

	int_t maximumScrollY()
	{
		return AchievementList::maxDisplayRow * 24 - 77;
	}
}

AchievementsScreen::AchievementsScreen(Minecraft &minecraft, StatFileWriter &stats)
	: Screen(minecraft), stats(stats)
{
	short centerX = 141;
	short centerY = 141;
	previousScrollX = scrollX = targetScrollX = AchievementList::openInventory->displayColumn * 24 - centerX / 2 - 12;
	previousScrollY = scrollY = targetScrollY = AchievementList::openInventory->displayRow * 24 - centerY / 2;
}

void AchievementsScreen::init()
{
	buttons.clear();
	buttons.push_back(Util::make_shared<SmallButton>(1, width / 2 + 24, height / 2 + 74, 80, 20, StatCollector::translate(u"gui.done")));
}

void AchievementsScreen::buttonClicked(Button &button)
{
	if (button.id == 1)
	{
		minecraft.setScreen(nullptr);
		minecraft.grabMouse();
	}
	Screen::buttonClicked(button);
}

void AchievementsScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
	if (eventKey == minecraft.options.keyInventory.key)
	{
		minecraft.setScreen(nullptr);
		minecraft.grabMouse();
	}
	else
	{
		Screen::keyPressed(eventCharacter, eventKey);
	}
}

void AchievementsScreen::tick()
{
	previousScrollX = scrollX;
	previousScrollY = scrollY;
	double deltaX = targetScrollX - scrollX;
	double deltaY = targetScrollY - scrollY;
	if (deltaX * deltaX + deltaY * deltaY < 4.0)
	{
		scrollX += deltaX;
		scrollY += deltaY;
	}
	else
	{
		scrollX += deltaX * 0.85;
		scrollY += deltaY * 0.85;
	}
}

void AchievementsScreen::render(int_t mouseX, int_t mouseY, float partialTick)
{
	if (lwjgl::Mouse::isButtonDown(0))
	{
		int_t panelX = (width - PANEL_WIDTH) / 2;
		int_t panelY = (height - PANEL_HEIGHT) / 2;
		int_t dragX = panelX + 8;
		int_t dragY = panelY + 17;
		if ((dragState == 0 || dragState == 1) && mouseX >= dragX && mouseX < dragX + MAP_WIDTH && mouseY >= dragY && mouseY < dragY + MAP_HEIGHT)
		{
			if (dragState == 0)
			{
				dragState = 1;
			}
			else
			{
				scrollX -= mouseX - lastMouseX;
				scrollY -= mouseY - lastMouseY;
				targetScrollX = previousScrollX = scrollX;
				targetScrollY = previousScrollY = scrollY;
			}

			lastMouseX = mouseX;
			lastMouseY = mouseY;
		}

		if (targetScrollX < minimumScrollX())
			targetScrollX = minimumScrollX();
		if (targetScrollY < minimumScrollY())
			targetScrollY = minimumScrollY();
		if (targetScrollX >= maximumScrollX())
			targetScrollX = maximumScrollX() - 1;
		if (targetScrollY >= maximumScrollY())
			targetScrollY = maximumScrollY() - 1;
	}
	else
	{
		dragState = 0;
	}

	renderBackground();
	renderAchievementMap(mouseX, mouseY, partialTick);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	renderTitle();
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
}

void AchievementsScreen::renderTitle()
{
	int_t panelX = (width - PANEL_WIDTH) / 2;
	int_t panelY = (height - PANEL_HEIGHT) / 2;
	font.draw(u"Achievements", panelX + 15, panelY + 5, 4210752);
}

void AchievementsScreen::drawHorizontalConnection(int_t x0, int_t x1, int_t y, int_t color)
{
	if (x1 < x0)
		std::swap(x0, x1);
	fill(x0, y, x1 + 1, y + 1, color);
}

void AchievementsScreen::drawVerticalConnection(int_t x, int_t y0, int_t y1, int_t color)
{
	if (y1 < y0)
		std::swap(y0, y1);
	fill(x, y0 + 1, x + 1, y1, color);
}

void AchievementsScreen::renderAchievementMap(int_t mouseX, int_t mouseY, float partialTick)
{
	int_t mapScrollX = Mth::floor(previousScrollX + (scrollX - previousScrollX) * partialTick);
	int_t mapScrollY = Mth::floor(previousScrollY + (scrollY - previousScrollY) * partialTick);
	mapScrollX = std::max(mapScrollX, minimumScrollX());
	mapScrollY = std::max(mapScrollY, minimumScrollY());
	if (mapScrollX >= maximumScrollX())
		mapScrollX = maximumScrollX() - 1;
	if (mapScrollY >= maximumScrollY())
		mapScrollY = maximumScrollY() - 1;

	int_t terrainTexture = minecraft.textures.loadTexture(u"/terrain.png");
	int_t backgroundTexture = minecraft.textures.loadTexture(u"/achievement/bg.png");
	int_t panelX = (width - PANEL_WIDTH) / 2;
	int_t panelY = (height - PANEL_HEIGHT) / 2;
	int_t mapX = panelX + 16;
	int_t mapY = panelY + 17;
	blitOffset = 0.0f;
	glDepthFunc(GL_GEQUAL);
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -200.0f);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_COLOR_MATERIAL);
	minecraft.textures.bind(terrainTexture);
	int_t tileColumn = (mapScrollX + 288) >> 4;
	int_t tileRow = (mapScrollY + 288) >> 4;
	int_t tileOffsetX = (mapScrollX + 288) % 16;
	int_t tileOffsetY = (mapScrollY + 288) % 16;
	Random random;

	for (int_t row = 0; row * 16 - tileOffsetY < MAP_HEIGHT; ++row)
	{
		float brightness = 0.6f - (tileRow + row) / 25.0f * 0.3f;
		glColor4f(brightness, brightness, brightness, 1.0f);
		for (int_t column = 0; column * 16 - tileOffsetX < MAP_WIDTH; ++column)
		{
			random.setSeed(1234 + tileColumn + column);
			random.nextInt();
			int_t depth = random.nextInt(1 + tileRow + row) + (tileRow + row) / 2;
			int_t texture = Tile::sand.tex;
			if (depth > 37 || tileRow + row == 35)
				texture = Tile::bedrock.tex;
			else if (depth == 22)
				texture = random.nextInt(2) == 0 ? Tile::diamondOre.tex : Tile::redstoneOre.tex;
			else if (depth == 10)
				texture = Tile::ironOre.tex;
			else if (depth == 8)
				texture = Tile::coalOre.tex;
			else if (depth > 4)
				texture = Tile::rock.tex;
			else if (depth > 0)
				texture = Tile::dirt.tex;

			blit(mapX + column * 16 - tileOffsetX, mapY + row * 16 - tileOffsetY,
				texture % 16 << 4, texture >> 4 << 4, 16, 16);
		}
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_TEXTURE_2D);

	for (Achievement *achievement : AchievementList::achievements)
	{
		if (achievement->parentAchievement == nullptr)
			continue;

		int_t x = achievement->displayColumn * 24 - mapScrollX + 11 + mapX;
		int_t y = achievement->displayRow * 24 - mapScrollY + 11 + mapY;
		int_t parentX = achievement->parentAchievement->displayColumn * 24 - mapScrollX + 11 + mapX;
		int_t parentY = achievement->parentAchievement->displayRow * 24 - mapScrollY + 11 + mapY;
		int_t color;
		bool unlocked = stats.hasAchievementUnlocked(*achievement);
		bool unlockable = stats.canUnlockAchievement(*achievement);
		int_t pulseAlpha = std::sin(System::currentTimeMillis() % 600L / 600.0 * 3.14159265358979323846 * 2.0) > 0.6 ? 255 : 130;
		if (unlocked)
			color = -9408400;
		else if (unlockable)
			color = static_cast<int_t>(65280U + (static_cast<uint_t>(pulseAlpha) << 24));
		else
			color = -16777216;

		drawHorizontalConnection(x, parentX, y, color);
		drawVerticalConnection(parentX, y, parentY, color);
	}

	Achievement *hovered = nullptr;
	static ItemRenderer itemRenderer(EntityRenderDispatcher::instance);
	glPushMatrix();
	glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
	Lighting::turnOn();
	glPopMatrix();
	glDisable(GL_LIGHTING);
	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_COLOR_MATERIAL);

	for (Achievement *achievement : AchievementList::achievements)
	{
		int_t relativeX = achievement->displayColumn * 24 - mapScrollX;
		int_t relativeY = achievement->displayRow * 24 - mapScrollY;
		if (relativeX < -24 || relativeY < -24 || relativeX > MAP_WIDTH || relativeY > MAP_HEIGHT)
			continue;

		bool unlocked = stats.hasAchievementUnlocked(*achievement);
		bool unlockable = stats.canUnlockAchievement(*achievement);
		if (unlocked)
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		else if (unlockable)
		{
			float brightness = std::sin(System::currentTimeMillis() % 600L / 600.0 * 3.14159265358979323846 * 2.0) < 0.6 ? 0.6f : 0.8f;
			glColor4f(brightness, brightness, brightness, 1.0f);
		}
		else
		{
			glColor4f(0.3f, 0.3f, 0.3f, 1.0f);
		}

		minecraft.textures.bind(backgroundTexture);
		int_t iconX = mapX + relativeX;
		int_t iconY = mapY + relativeY;
		blit(iconX - 2, iconY - 2, achievement->isSpecial() ? 26 : 0, 202, 26, 26);
		if (!unlockable)
		{
			glColor4f(0.1f, 0.1f, 0.1f, 1.0f);
			itemRenderer.renderWithColor = false;
		}

		glEnable(GL_LIGHTING);
		glEnable(GL_CULL_FACE);
		ItemInstance item = achievement->item;
		itemRenderer.renderGuiItem(font, minecraft.textures, item, iconX + 3, iconY + 3);
		glDisable(GL_LIGHTING);
		if (!unlockable)
			itemRenderer.renderWithColor = true;

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		if (mouseX >= mapX && mouseY >= mapY && mouseX < mapX + MAP_WIDTH && mouseY < mapY + MAP_HEIGHT &&
			mouseX >= iconX && mouseX <= iconX + 22 && mouseY >= iconY && mouseY <= iconY + 22)
			hovered = achievement;
	}

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	minecraft.textures.bind(backgroundTexture);
	blit(panelX, panelY, 0, 0, PANEL_WIDTH, PANEL_HEIGHT);
	glPopMatrix();
	blitOffset = 0.0f;
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	Screen::render(mouseX, mouseY, partialTick);

	if (hovered != nullptr)
	{
		jstring name = hovered->statName;
		jstring description = hovered->getDescription();
		int_t tooltipX = mouseX + 12;
		int_t tooltipY = mouseY - 4;
		bool unlockable = stats.canUnlockAchievement(*hovered);
		if (unlockable)
		{
			int_t tooltipWidth = std::max(font.width(name), 120);
			int_t tooltipHeight = font.wordWrapHeight(description, tooltipWidth);
			if (stats.hasAchievementUnlocked(*hovered))
				tooltipHeight += 12;
			fillGradient(tooltipX - 3, tooltipY - 3, tooltipX + tooltipWidth + 3, tooltipY + tooltipHeight + 15, -1073741824, -1073741824);
			font.drawWordWrap(description, tooltipX, tooltipY + 12, tooltipWidth, -6250336);
			if (stats.hasAchievementUnlocked(*hovered))
				font.drawShadow(StatCollector::translate(u"achievement.taken"), tooltipX, tooltipY + tooltipHeight + 4, -7302913);
		}
		else
		{
			int_t tooltipWidth = std::max(font.width(name), 120);
			jstring requires = StatCollector::translate(u"achievement.requires", hovered->parentAchievement->statName);
			int_t tooltipHeight = font.wordWrapHeight(requires, tooltipWidth);
			fillGradient(tooltipX - 3, tooltipY - 3, tooltipX + tooltipWidth + 3, tooltipY + tooltipHeight + 15, -1073741824, -1073741824);
			font.drawWordWrap(requires, tooltipX, tooltipY + 12, tooltipWidth, -9416624);
		}

		int_t titleColor = unlockable ? (hovered->isSpecial() ? -128 : -1) : (hovered->isSpecial() ? -8355776 : -8355712);
		font.drawShadow(name, tooltipX, tooltipY, titleColor);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	Lighting::turnOff();
}
