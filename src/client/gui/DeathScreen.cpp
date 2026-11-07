#include "client/gui/DeathScreen.h"

#include "client/Minecraft.h"
#include "client/gui/Button.h"
#include "client/title/TitleScreen.h"
#include "client/locale/Language.h"
#include "java/File.h"
#include "world/level/Level.h"

#include "java/String.h"

#include "OpenGL.h"

DeathScreen::DeathScreen(Minecraft &minecraft) : Screen(minecraft)
{
}

void DeathScreen::init()
{
	Language &language = Language::getInstance();
	buttons.clear();
	if (minecraft.level != nullptr && minecraft.level->hardcore)
	{
		buttons.push_back(Util::make_shared<Button>(1, width / 2 - 100, height / 4 + 96, language.getElement(u"deathScreen.deleteWorld")));
	}
	else
	{
		buttons.push_back(Util::make_shared<Button>(1, width / 2 - 100, height / 4 + 72, language.getElement(u"deathScreen.respawn")));
		buttons.push_back(Util::make_shared<Button>(2, width / 2 - 100, height / 4 + 96, language.getElement(u"deathScreen.titleScreen")));
	}
}

void DeathScreen::keyPressed(char_t, int_t)
{
}

void DeathScreen::buttonClicked(Button &button)
{
	if (button.id == 1)
	{
		if (minecraft.level != nullptr && minecraft.level->hardcore)
		{
			// 1.0 hardcore: one life, the world goes away
			jstring folder = minecraft.level->name;
			minecraft.setLevel(nullptr, u"Deleting world");
			std::unique_ptr<File> saves(File::open(*minecraft.getWorkingDirectory(), u"saves"));
			Level::deleteLevel(*saves, folder);
			minecraft.setScreen(Util::make_shared<TitleScreen>(minecraft));
		}
		else
		{
			minecraft.player->respawn();
			minecraft.setScreen(nullptr);
		}
	}
	else if (button.id == 2)
	{
		minecraft.setLevel(nullptr);
		minecraft.setScreen(Util::make_shared<TitleScreen>(minecraft));
	}
}

void DeathScreen::render(int_t xm, int_t ym, float a)
{
	Language &language = Language::getInstance();
	bool hardcore = minecraft.level != nullptr && minecraft.level->hardcore;
	fillGradient(0, 0, width, height, 0x60500000, 0xA0803030);

	glPushMatrix();
	glScalef(2.0f, 2.0f, 2.0f);
	drawCenteredString(font, language.getElement(hardcore ? u"deathScreen.title.hardcore" : u"deathScreen.title"), width / 2 / 2, 30, 0xFFFFFF);
	glPopMatrix();

	if (hardcore)
		drawCenteredString(font, language.getElement(u"deathScreen.hardcoreInfo"), width / 2, 144, 0xFFFFFF);

	jstring scoreText = language.getElement(u"deathScreen.score") + u": &e" + String::toString(minecraft.player->score);
	drawCenteredString(font, scoreText, width / 2, 100, 0xFFFFFF);

	Screen::render(xm, ym, a);
}

bool DeathScreen::isPauseScreen()
{
	return false;
}
