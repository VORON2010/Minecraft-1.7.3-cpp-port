#include "client/gui/ConnectFailedScreen.h"

#include <stdexcept>

#include "client/Minecraft.h"
#include "client/locale/Language.h"
#include "client/title/TitleScreen.h"

namespace
{
	jstring translate(const jstring &key)
	{
		return Language::getInstance().getElement(key);
	}

	jstring translateFormat(const jstring &key, const std::vector<jstring> &arguments)
	{
		const jstring format = translate(key);
		jstring result;
		size_t nextArgument = 0;

		for (size_t i = 0; i < format.size(); ++i)
		{
			if (format[i] != u'%' || i + 1 >= format.size())
			{
				result.push_back(format[i]);
				continue;
			}

			if (format[i + 1] == u'%')
			{
				result.push_back(u'%');
				++i;
				continue;
			}

			size_t argument = nextArgument;
			bool explicitArgument = false;
			size_t end = i + 1;
			if (format[end] >= u'1' && format[end] <= u'9')
			{
				size_t number = 0;
				while (end < format.size() && format[end] >= u'0' && format[end] <= u'9')
				{
					number = number * 10 + static_cast<size_t>(format[end] - u'0');
					++end;
				}
				if (end < format.size() && format[end] == u'$')
				{
					argument = number - 1;
					explicitArgument = true;
					++end;
				}
				else
				{
					result.push_back(format[i]);
					continue;
				}
			}

			if (end < format.size() && format[end] == u's')
			{
				if (argument >= arguments.size())
					throw std::invalid_argument("Missing format argument");
				result += arguments[argument];
				if (!explicitArgument)
					++nextArgument;
				i = end;
				continue;
			}

			result.push_back(format[i]);
		}

		return result;
	}
}

ConnectFailedScreen::ConnectFailedScreen(Minecraft &minecraft, const jstring &messageKey,
	const jstring &detailKey)
	: Screen(minecraft), errorMessage(translate(messageKey)), errorDetail(translate(detailKey))
{
}

ConnectFailedScreen::ConnectFailedScreen(Minecraft &minecraft, const jstring &messageKey,
	const jstring &detailKey, const jstring &argument)
	: Screen(minecraft), errorMessage(translate(messageKey)),
	  errorDetail(translateFormat(detailKey, std::vector<jstring>{argument}))
{
}

ConnectFailedScreen::ConnectFailedScreen(Minecraft &minecraft, const jstring &messageKey,
	const jstring &detailKey, const std::vector<jstring> &arguments)
	: Screen(minecraft), errorMessage(translate(messageKey)),
	  errorDetail(translateFormat(detailKey, arguments))
{
}

void ConnectFailedScreen::tick()
{
}

void ConnectFailedScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
}

void ConnectFailedScreen::init()
{
	buttons.clear();
	buttons.push_back(Util::make_shared<Button>(0, width / 2 - 100, height / 4 + 120 + 12,
		translate(u"gui.toMenu")));
}

void ConnectFailedScreen::buttonClicked(Button &button)
{
	if (button.id == 0)
		minecraft.setScreen(Util::make_shared<TitleScreen>(minecraft));
}

void ConnectFailedScreen::render(int_t xm, int_t ym, float a)
{
	renderBackground();
	drawCenteredString(font, errorMessage, width / 2, height / 2 - 50, 0xFFFFFF);
	drawCenteredString(font, errorDetail, width / 2, height / 2 - 10, 0xFFFFFF);
	Screen::render(xm, ym, a);
}
