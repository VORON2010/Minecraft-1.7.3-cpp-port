#pragma once

#include "client/gui/Screen.h"

class GuiSlot;

class TexturePackSelectScreen : public Screen
{
private:
	std::shared_ptr<Screen> lastScreen;
	std::shared_ptr<GuiSlot> texturePackList;
	int_t refreshTicks = -1;

public:
	TexturePackSelectScreen(Minecraft &minecraft, std::shared_ptr<Screen> lastScreen);

	void init() override;
	void tick() override;

protected:
	void buttonClicked(Button &button) override;
	void mouseScrolled(int_t x, int_t y, int_t scrollAmount) override;

public:
	void render(int_t xm, int_t ym, float a) override;
};
