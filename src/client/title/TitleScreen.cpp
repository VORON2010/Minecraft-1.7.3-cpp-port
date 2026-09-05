#include "client/title/TitleScreen.h"

#include <chrono>
#include <cctype>
#include <ctime>
#include "client/renderer/Tesselator.h"
#include "client/gui/Screen.h"

#include "client/Minecraft.h"
#include "client/gui/MultiplayerScreen.h"
#include "client/gui/OptionsScreen.h"
#include "client/gui/SelectWorldScreen.h"
#include "client/locale/Language.h"
#include "client/skins/TexturePackSelectScreen.h"

#include "java/Resource.h"

#include "util/Mth.h"

#include "OpenGL.h"

Random TitleScreen::random;

TitleScreen::TitleScreen(Minecraft &minecraft) : Screen(minecraft)
{
	std::vector<std::string> splashes;
	std::unique_ptr<std::istream> is(Resource::getResource(u"/title/splashes.txt"));
	std::string line;
	while (std::getline(*is, line))
	{
		while (!line.empty() && std::isspace(static_cast<unsigned char>(line.back())))
			line.pop_back();
		while (!line.empty() && std::isspace(static_cast<unsigned char>(line.front())))
			line.erase(0, 1);
		if (!line.empty())
			splashes.push_back(line);
	}

	if (!splashes.empty())
		splash = String::fromUTF8(splashes[random.nextInt(static_cast<int_t>(splashes.size()))]);
}

void TitleScreen::init()
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	std::tm *tm = std::localtime(&now_c);
	if (tm != nullptr)
	{
		if (tm->tm_mon + 1 == 11 && tm->tm_mday == 9)
			splash = u"Happy birthday, ez!";
		else if (tm->tm_mon + 1 == 6 && tm->tm_mday == 1)
			splash = u"Happy birthday, Notch!";
		else if (tm->tm_mon + 1 == 12 && tm->tm_mday == 24)
			splash = u"Merry X-mas!";
		else if (tm->tm_mon + 1 == 1 && tm->tm_mday == 1)
			splash = u"Happy new year!";
	}

	Language &language = Language::getInstance();
	int_t y = height / 4 + 48;
	buttons.push_back(Util::make_shared<Button>(1, width / 2 - 100, y, language.getElement(u"menu.singleplayer")));
	buttons.push_back(Util::make_shared<Button>(2, width / 2 - 100, y + 24, language.getElement(u"menu.multiplayer")));
	buttons.push_back(Util::make_shared<Button>(3, width / 2 - 100, y + 48, language.getElement(u"menu.mods")));
	buttons.push_back(Util::make_shared<Button>(0, width / 2 - 100, y + 84, 98, 20, language.getElement(u"menu.options")));
	buttons.push_back(Util::make_shared<Button>(4, width / 2 + 2, y + 84, 98, 20, language.getElement(u"menu.quit")));
	if (minecraft.user == nullptr)
		buttons[1]->active = false;
}


void TitleScreen::keyPressed(char_t, int_t)
{
}

void TitleScreen::buttonClicked(Button &button)
{
	if (!button.active)
		return;

	if (button.id == 0)
		minecraft.setScreen(Util::make_shared<OptionsScreen>(minecraft, minecraft.screen, minecraft.options));
	else if (button.id == 1)
		minecraft.setScreen(Util::make_shared<SelectWorldScreen>(minecraft, minecraft.screen));
	else if (button.id == 2)
		minecraft.setScreen(Util::make_shared<MultiplayerScreen>(minecraft, minecraft.screen));
	else if (button.id == 3)
		minecraft.setScreen(Util::make_shared<TexturePackSelectScreen>(minecraft, minecraft.screen));
	else if (button.id == 4)
		minecraft.stop();
}

void TitleScreen::render(int_t xm, int_t ym, float a)
{
	renderBackground();

	int_t logoWidth = 274;
	int_t logoHeight = 73; // 274 * (269 / 1003)
	int_t logoX = width / 2 - logoWidth / 2;
	int_t logoY = 30;
	glBindTexture(GL_TEXTURE_2D, minecraft.textures.loadTexture(u"/title/voron_logo.png"));
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	Tesselator &t = Tesselator::instance;
	t.begin();
	t.vertexUV(logoX, logoY + logoHeight, 0.0, 0.0, 1.0);
	t.vertexUV(logoX + logoWidth, logoY + logoHeight, 0.0, 1.0, 1.0);
	t.vertexUV(logoX + logoWidth, logoY, 0.0, 1.0, 0.0);
	t.vertexUV(logoX, logoY, 0.0, 0.0, 0.0);
	t.end();

	glPushMatrix();
	glTranslatef(static_cast<float>(width / 2 + 90), 70.0f, 0.0f);
	glRotatef(-20.0f, 0.0f, 0.0f, 1.0f);
	float scale = 1.8f - Mth::abs(Mth::sin(static_cast<float>(System::currentTimeMillis() % 1000L) / 1000.0f * Mth::PI * 2.0f) * 0.1f);
	scale = scale * 100.0f / (font.width(splash) + 32);
	glScalef(scale, scale, scale);
	drawCenteredString(font, splash, 0, -8, 0xFFFF00);
	glPopMatrix();

	drawString(font, Minecraft::VERSION_STRING, 2, 2, 0x505050);
	jstring copyright = u"VORON";
	drawString(font, copyright, width - font.width(copyright) - 2, height - 10, 0xFFFFFF);
	Screen::render(xm, ym, a);
}
