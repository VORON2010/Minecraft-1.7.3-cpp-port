#pragma once

#include "client/gui/Screen.h"

class Options;
class GuiTextField;

class OptionsScreen: public Screen
{
private:
	std::shared_ptr<Screen> lastScreen;

protected:
	jstring title = u"Options";

private:
	Options &options;
	std::unique_ptr<GuiTextField> usernameField;

	void saveUsername();

public:
	OptionsScreen(Minecraft &minecraft, std::shared_ptr<Screen> lastScreen, Options &options);
	~OptionsScreen() override;

	void init() override;
	void removed() override;
	void tick() override;

protected:
	void keyPressed(char_t eventCharacter, int_t eventKey) override;
	void mouseClicked(int_t x, int_t y, int_t buttonNum) override;
	void buttonClicked(Button &button) override;

public:
	void render(int_t xm, int_t ym, float a) override;
};
