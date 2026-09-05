#pragma once

#include "client/gui/Screen.h"

class GuiTextField;

class CreateWorldScreen : public Screen
{
private:
	std::shared_ptr<Screen> lastScreen;
	std::shared_ptr<GuiTextField> worldNameField;
	std::shared_ptr<GuiTextField> seedField;
	jstring folderName;
	bool createClicked = false;

public:
	CreateWorldScreen(Minecraft &minecraft, std::shared_ptr<Screen> lastScreen);

	void init() override;
	void tick() override;
	void removed() override;
	void selectNextField() override;

protected:
	void buttonClicked(Button &button) override;
	void keyPressed(char_t eventCharacter, int_t eventKey) override;
	void mouseClicked(int_t x, int_t y, int_t buttonNum) override;

public:
	void render(int_t xm, int_t ym, float a) override;

private:
	void updateFolderName();
	void updateCreateButton();
	long_t parseSeed() const;
};
