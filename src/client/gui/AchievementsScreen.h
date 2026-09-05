#pragma once

#include "client/gui/Screen.h"

class Achievement;
class StatFileWriter;

class AchievementsScreen : public Screen
{
private:
	StatFileWriter &stats;
	int_t lastMouseX = 0;
	int_t lastMouseY = 0;
	double previousScrollX;
	double previousScrollY;
	double scrollX;
	double scrollY;
	double targetScrollX;
	double targetScrollY;
	int_t dragState = 0;

	void drawHorizontalConnection(int_t x0, int_t x1, int_t y, int_t color);
	void drawVerticalConnection(int_t x, int_t y0, int_t y1, int_t color);
	void renderTitle();
	void renderAchievementMap(int_t mouseX, int_t mouseY, float partialTick);

protected:
	void keyPressed(char_t eventCharacter, int_t eventKey) override;
	void buttonClicked(Button &button) override;

public:
	AchievementsScreen(Minecraft &minecraft, StatFileWriter &stats);

	void init() override;
	void tick() override;
	void render(int_t mouseX, int_t mouseY, float partialTick) override;
};
