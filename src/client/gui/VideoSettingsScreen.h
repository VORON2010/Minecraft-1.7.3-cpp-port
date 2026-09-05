#pragma once

#include "client/gui/Screen.h"

class Options;

class VideoSettingsScreen : public Screen
{
private:
	std::shared_ptr<Screen> lastScreen;
	Options &options;
	jstring title = u"Video Settings";

public:
	VideoSettingsScreen(Minecraft &minecraft, std::shared_ptr<Screen> lastScreen, Options &options);

	void init() override;

protected:
	void buttonClicked(Button &button) override;

public:
	void render(int_t xm, int_t ym, float a) override;
};
