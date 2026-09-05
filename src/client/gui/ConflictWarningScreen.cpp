#include "client/gui/ConflictWarningScreen.h"

#include "client/Minecraft.h"
#include "client/title/TitleScreen.h"

ConflictWarningScreen::ConflictWarningScreen(Minecraft &minecraft) : Screen(minecraft)
{
}

void ConflictWarningScreen::init()
{
	buttons.clear();
	buttons.push_back(Util::make_shared<Button>(0, width / 2 - 100, height / 4 + 120 + 12, u"Back to title screen"));
}

void ConflictWarningScreen::buttonClicked(Button &button)
{
	if (button.id == 0)
		minecraft.setScreen(Util::make_shared<TitleScreen>(minecraft));
}

void ConflictWarningScreen::render(int_t xm, int_t ym, float a)
{
	renderBackground();
	drawCenteredString(font, u"Level save conflict", width / 2, height / 4 - 60 + 20, 0xFFFFFF);
	drawString(font, u"Minecraft detected a conflict in the level save data.", width / 2 - 140, height / 4 - 60 + 60 + 0, 0xA0A0A0);
	drawString(font, u"This could be caused by two copies of the game", width / 2 - 140, height / 4 - 60 + 60 + 18, 0xA0A0A0);
	drawString(font, u"accessing the same level.", width / 2 - 140, height / 4 - 60 + 60 + 27, 0xA0A0A0);
	drawString(font, u"To prevent level corruption, the current game has quit.", width / 2 - 140, height / 4 - 60 + 60 + 45, 0xA0A0A0);
	Screen::render(xm, ym, a);
}
