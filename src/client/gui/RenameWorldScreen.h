#pragma once

#include "client/gui/Screen.h"

class GuiTextField;

class RenameWorldScreen : public Screen
{
private:
	std::shared_ptr<Screen> lastScreen;
	jstring folderName;
	jstring currentName;
	std::shared_ptr<GuiTextField> nameField;

public:
	RenameWorldScreen(Minecraft &minecraft, std::shared_ptr<Screen> lastScreen, const jstring &folderName, const jstring &currentName);

	void init() override;
	void tick() override;
	void removed() override;

protected:
	void buttonClicked(Button &button) override;
	void keyPressed(char_t eventCharacter, int_t eventKey) override;
	void mouseClicked(int_t x, int_t y, int_t buttonNum) override;

public:
	void render(int_t xm, int_t ym, float a) override;

private:
	void updateRenameButton();
};
