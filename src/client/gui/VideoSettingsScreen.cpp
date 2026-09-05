#include "client/gui/VideoSettingsScreen.h"

#include "client/Minecraft.h"
#include "client/OpenGLCapabilities.h"
#include "client/Options.h"
#include "client/locale/Language.h"

#include "client/gui/SmallButton.h"
#include "client/gui/SlideButton.h"

namespace
{

Options::Option::Element *const VIDEO_OPTIONS[] = {
	&Options::Option::GRAPHICS,
	&Options::Option::RENDER_DISTANCE,
	&Options::Option::AMBIENT_OCCLUSION,
	&Options::Option::LIMIT_FRAMERATE,
	&Options::Option::ANAGLYPH,
	&Options::Option::VIEW_BOBBING,
	&Options::Option::GUI_SCALE,
	&Options::Option::ADVANCED_OPENGL,
};

void addOptionButton(std::vector<std::shared_ptr<Button>> &buttons, int_t x, int_t y, int_t id, Options::Option::Element &option, Options &options)
{
	if (option.isProgress)
		buttons.push_back(Util::make_shared<SlideButton>(id, x, y, &option, options.getMessage(option), options.getProgressValue(option)));
	else
		buttons.push_back(Util::make_shared<SmallButton>(id, x, y, &option, options.getMessage(option)));
}

}

VideoSettingsScreen::VideoSettingsScreen(Minecraft &minecraft, std::shared_ptr<Screen> lastScreen, Options &options)
	: Screen(minecraft), lastScreen(lastScreen), options(options)
{

}

void VideoSettingsScreen::init()
{
	Language &language = Language::getInstance();
	title = language.getElement(u"options.videoTitle");

	for (int_t i = 0; i < Util::size(VIDEO_OPTIONS); i++)
	{
		Options::Option::Element &option = *VIDEO_OPTIONS[i];
		addOptionButton(buttons, width / 2 - 155 + i % 2 * 160, height / 6 + 24 * (i >> 1), i, option, options);
		if (&option == &Options::Option::ADVANCED_OPENGL && !OpenGLCapabilities::hasOcclusionChecks())
			buttons.back()->active = false;
	}

	buttons.push_back(Util::make_shared<Button>(200, width / 2 - 100, height / 6 + 168, language.getElement(u"gui.done")));
}

void VideoSettingsScreen::buttonClicked(Button &button)
{
	if (!button.active)
		return;

	if (button.id < Util::size(VIDEO_OPTIONS) && button.isSmallButton())
	{
		auto &smallButton = reinterpret_cast<SmallButton &>(button);
		options.toggle(*smallButton.getOption(), 1);
		minecraft.setScreen(Util::make_shared<VideoSettingsScreen>(minecraft, lastScreen, options));
		return;
	}

	if (button.id == 200)
	{
		minecraft.options.save();
		minecraft.setScreen(lastScreen);
	}
}

void VideoSettingsScreen::render(int_t xm, int_t ym, float a)
{
	renderBackground();
	drawCenteredString(font, title, width / 2, 20, 0xFFFFFF);
	Screen::render(xm, ym, a);
}
