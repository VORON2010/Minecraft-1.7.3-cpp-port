#pragma once

#include "client/gui/Screen.h"

class DeathScreen : public Screen
{
public:
	DeathScreen(Minecraft &minecraft);

	void init() override;

protected:
	void keyPressed(char_t eventCharacter, int_t eventKey) override;
	void buttonClicked(Button &button) override;

public:
	void render(int_t xm, int_t ym, float a) override;
	bool isPauseScreen() override;
};
