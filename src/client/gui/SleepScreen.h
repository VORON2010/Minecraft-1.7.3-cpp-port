#pragma once

#include "client/gui/ChatScreen.h"

class SleepScreen : public ChatScreen
{
public:
	SleepScreen(Minecraft &minecraft);

	void init() override;
	void removed() override;
	void render(int_t xm, int_t ym, float a) override;

protected:
	void keyPressed(char_t eventCharacter, int_t eventKey) override;
	void buttonClicked(Button &button) override;

private:
	void stopSleeping();
};
