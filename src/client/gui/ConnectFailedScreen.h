#pragma once

#include <vector>

#include "client/gui/Screen.h"

class ConnectFailedScreen : public Screen
{
private:
	jstring errorMessage;
	jstring errorDetail;

public:
	ConnectFailedScreen(Minecraft &minecraft, const jstring &messageKey, const jstring &detailKey);
	ConnectFailedScreen(Minecraft &minecraft, const jstring &messageKey, const jstring &detailKey,
		const jstring &argument);
	ConnectFailedScreen(Minecraft &minecraft, const jstring &messageKey, const jstring &detailKey,
		const std::vector<jstring> &arguments);

	void tick() override;

protected:
	void keyPressed(char_t eventCharacter, int_t eventKey) override;

public:
	void init() override;

protected:
	void buttonClicked(Button &button) override;

public:
	void render(int_t xm, int_t ym, float a) override;
};
