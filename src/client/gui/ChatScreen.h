#pragma once

#include "client/gui/Screen.h"

class ChatScreen : public Screen
{
public:
	ChatScreen(Minecraft &minecraft);

	void init() override;
	void removed() override;
	void tick() override;
	void render(int_t xm, int_t ym, float a) override;
	void keyPressed(char_t eventCharacter, int_t eventKey) override;

protected:
	jstring message;

private:
	int_t frame = 0;
};
