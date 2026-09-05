#include "client/gui/OptionsScreen.h"

#include "client/Minecraft.h"
#include "client/Options.h"
#include "client/User.h"
#include "client/locale/Language.h"

#include "client/gui/ControlsScreen.h"
#include "client/gui/GuiTextField.h"
#include "client/gui/VideoSettingsScreen.h"

#include "client/gui/SmallButton.h"
#include "client/gui/SlideButton.h"

#include "lwjgl/Keyboard.h"

namespace
{

Options::Option::Element *const OPTION_SCREEN_OPTIONS[] = {
	&Options::Option::MUSIC,
	&Options::Option::SOUND,
	&Options::Option::INVERT_MOUSE,
	&Options::Option::SENSITIVITY,
	&Options::Option::DIFFICULTY,
};

void addOptionButton(std::vector<std::shared_ptr<Button>> &buttons, int_t x, int_t y, int_t id, Options::Option::Element &option, Options &options)
{
	if (option.isProgress)
		buttons.push_back(Util::make_shared<SlideButton>(id, x, y, &option, options.getMessage(option), options.getProgressValue(option)));
	else
		buttons.push_back(Util::make_shared<SmallButton>(id, x, y, &option, options.getMessage(option)));
}

}

OptionsScreen::OptionsScreen(Minecraft &minecraft, std::shared_ptr<Screen> lastScreen, Options &options) : Screen(minecraft), lastScreen(lastScreen), options(options)
{

}

OptionsScreen::~OptionsScreen() = default;

void OptionsScreen::init()
{
	Language &language = Language::getInstance();
	lwjgl::Keyboard::enableRepeatEvents(true);
	title = language.getElement(u"options.title");

	for (int_t i = 0; i < Util::size(OPTION_SCREEN_OPTIONS); i++)
	{
		Options::Option::Element &option = *OPTION_SCREEN_OPTIONS[i];
		addOptionButton(buttons, width / 2 - 155 + i % 2 * 160, height / 6 + 24 * (i >> 1), i, option, options);
	}
	usernameField = std::make_unique<GuiTextField>(*this, font, width / 2 - 100,
		height / 6 + 84, 200, 20, options.username);
	usernameField->setMaxStringLength(16);

	buttons.push_back(Util::make_shared<Button>(101, width / 2 - 100, height / 6 + 108, language.getElement(u"options.video")));
	buttons.push_back(Util::make_shared<Button>(100, width / 2 - 100, height / 6 + 132, language.getElement(u"options.controls")));
	buttons.push_back(Util::make_shared<Button>(200, width / 2 - 100, height / 6 + 168, language.getElement(u"gui.done")));
}

void OptionsScreen::saveUsername()
{
	options.setUsername(usernameField->getText());
	usernameField->setText(options.username);
	minecraft.user->name = options.username;
	options.save();
}

void OptionsScreen::removed()
{
	lwjgl::Keyboard::enableRepeatEvents(false);
	saveUsername();
}

void OptionsScreen::tick()
{
	usernameField->updateCursorCounter();
}

void OptionsScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
	usernameField->textboxKeyTyped(eventCharacter, eventKey);
	Screen::keyPressed(eventCharacter, eventKey);
}

void OptionsScreen::mouseClicked(int_t x, int_t y, int_t buttonNum)
{
	Screen::mouseClicked(x, y, buttonNum);
	usernameField->mouseClicked(x, y, buttonNum);
}

void OptionsScreen::buttonClicked(Button &button)
{
	if (!button.active)
		return;

	if (button.id < Util::size(OPTION_SCREEN_OPTIONS) && button.isSmallButton())
	{
		auto &smallButton = reinterpret_cast<SmallButton &>(button);
		options.toggle(*smallButton.getOption(), 1);
		smallButton.msg = options.getMessage(*smallButton.getOption());
		return;
	}

	if (button.id == 101)
	{
		minecraft.options.save();
		minecraft.setScreen(Util::make_shared<VideoSettingsScreen>(minecraft, minecraft.screen, options));
	}
	else if (button.id == 100)
	{
		minecraft.options.save();
		minecraft.setScreen(Util::make_shared<ControlsScreen>(minecraft, minecraft.screen, options));
	}
	else if (button.id == 200)
	{
		minecraft.options.save();
		minecraft.setScreen(lastScreen);
	}
}

void OptionsScreen::render(int_t xm, int_t ym, float a)
{
	Language &language = Language::getInstance();
	renderBackground();
	drawCenteredString(font, title, width / 2, 20, 0xFFFFFF);
	drawString(font, language.getElement(u"options.username"), width / 2 - 100,
		height / 6 + 72, 0xA0A0A0);
	usernameField->drawTextBox();
	Screen::render(xm, ym, a);
}
