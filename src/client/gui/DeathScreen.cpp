#include "client/gui/DeathScreen.h"

#include "client/Minecraft.h"
#include "client/gui/Button.h"
#include "client/title/TitleScreen.h"

#include "java/String.h"

#include "OpenGL.h"

DeathScreen::DeathScreen(Minecraft &minecraft) : Screen(minecraft)
{
}

void DeathScreen::init()
{
	buttons.clear();
	buttons.push_back(Util::make_shared<Button>(1, width / 2 - 100, height / 4 + 72, u"Respawn"));
	buttons.push_back(Util::make_shared<Button>(2, width / 2 - 100, height / 4 + 96, u"Title menu"));
}


void DeathScreen::keyPressed(char_t, int_t)
{
}

void DeathScreen::buttonClicked(Button &button)
{
	if (button.id == 1)
	{
		minecraft.player->respawn();
		minecraft.setScreen(nullptr);
	}
	else if (button.id == 2)
	{
		minecraft.setLevel(nullptr);
		minecraft.setScreen(Util::make_shared<TitleScreen>(minecraft));
	}
}

void DeathScreen::render(int_t xm, int_t ym, float a)
{
	fillGradient(0, 0, width, height, 0x60500000, 0xA0803030);

	glPushMatrix();
	glScalef(2.0f, 2.0f, 2.0f);
	drawCenteredString(font, u"Game over!", width / 4, 30, 0xFFFFFF);
	glPopMatrix();

	jstring scoreText = u"Score: &e" + String::toString(minecraft.player->score);
	drawCenteredString(font, scoreText, width / 2, 100, 0xFFFFFF);

	Screen::render(xm, ym, a);
}

bool DeathScreen::isPauseScreen()
{
	return false;
}
