#include "client/gui/MultiplayerScreen.h"

#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

#include "client/Minecraft.h"
#include "client/gui/ConnectingScreen.h"
#include "client/locale/Language.h"
#include "java/String.h"
#include "lwjgl/Keyboard.h"

namespace
{

std::vector<jstring> splitOnColon(const jstring &text)
{
	if (text.empty())
		return {u""};

	std::vector<jstring> parts;
	size_t start = 0;
	for (size_t i = 0; i < text.size(); ++i)
	{
		if (text[i] != u':')
			continue;
		parts.push_back(text.substr(start, i - start));
		start = i + 1;
	}
	parts.push_back(text.substr(start));

	while (!parts.empty() && parts.back().empty())
		parts.pop_back();
	return parts;
}

}

MultiplayerScreen::MultiplayerScreen(Minecraft &minecraft, std::shared_ptr<Screen> parentScreen)
	: Screen(minecraft), parentScreen(std::move(parentScreen))
{
}

void MultiplayerScreen::init()
{
	Language &language = Language::getInstance();
	lwjgl::Keyboard::enableRepeatEvents(true);
	buttons.clear();
	buttons.push_back(Util::make_shared<Button>(0, width / 2 - 100, height / 4 + 96 + 12,
		language.getElement(u"multiplayer.connect")));
	buttons.push_back(Util::make_shared<Button>(1, width / 2 - 100, height / 4 + 120 + 12,
		language.getElement(u"gui.cancel")));

	jstring address = replace(minecraft.options.lastMpIp, u'_', u':');
	buttons[0]->active = !address.empty();
	serverAddress = std::make_unique<GuiTextField>(*this, font, width / 2 - 100,
		height / 4 - 10 + 50 + 18, 200, 20, address);
	serverAddress->isFocused = true;
	serverAddress->setMaxStringLength(128);
}

void MultiplayerScreen::removed()
{
	lwjgl::Keyboard::enableRepeatEvents(false);
}

void MultiplayerScreen::tick()
{
	serverAddress->updateCursorCounter();
}

void MultiplayerScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
	serverAddress->textboxKeyTyped(eventCharacter, eventKey);
	
	if (eventCharacter == u'\r' || eventKey == lwjgl::Keyboard::KEY_RETURN || eventKey == lwjgl::Keyboard::KEY_NUMPADENTER)
		buttonClicked(*buttons[0]);

	buttons[0]->active = !serverAddress->getText().empty();
}

void MultiplayerScreen::mouseClicked(int_t x, int_t y, int_t buttonNum)
{
	Screen::mouseClicked(x, y, buttonNum);
	serverAddress->mouseClicked(x, y, buttonNum);
}

void MultiplayerScreen::buttonClicked(Button &button)
{
	if (!button.active)
		return;

	if (button.id == 1)
	{
		minecraft.setScreen(parentScreen);
	}
	else if (button.id == 0)
	{
		jstring addressText = trim(serverAddress->getText());
		minecraft.options.lastMpIp = replace(addressText, u':', u'_');
		minecraft.options.save();

		ServerAddress address = parseServerAddress(addressText);
		minecraft.setScreen(Util::make_shared<ConnectingScreen>(minecraft, address.host, address.port));
	}
}

void MultiplayerScreen::render(int_t xm, int_t ym, float a)
{
	Language &language = Language::getInstance();
	renderBackground();
	drawCenteredString(font, language.getElement(u"multiplayer.title"), width / 2,
		height / 4 - 60 + 20, 0xFFFFFF);
	drawString(font, language.getElement(u"multiplayer.info1"), width / 2 - 140,
		height / 4 - 60 + 60, 0xA0A0A0);
	drawString(font, language.getElement(u"multiplayer.info2"), width / 2 - 140,
		height / 4 - 60 + 60 + 9, 0xA0A0A0);
	drawString(font, language.getElement(u"multiplayer.ipinfo"), width / 2 - 140,
		height / 4 - 60 + 60 + 36, 0xA0A0A0);
	serverAddress->drawTextBox();
	Screen::render(xm, ym, a);
}

jstring MultiplayerScreen::trim(const jstring &text)
{
	size_t first = 0;
	while (first < text.size() && text[first] <= u' ')
		++first;

	size_t last = text.size();
	while (last > first && text[last - 1] <= u' ')
		--last;
	return text.substr(first, last - first);
}

jstring MultiplayerScreen::replace(const jstring &text, char_t from, char_t to)
{
	jstring result = text;
	for (size_t i = 0; i < result.size(); ++i)
	{
		if (result[i] == from)
			result[i] = to;
	}
	return result;
}

int_t MultiplayerScreen::parseIntWithDefault(const jstring &text, int_t defaultValue)
{
	jstring value = trim(text);
	if (value.empty())
		return defaultValue;

	size_t index = 0;
	bool negative = false;
	int_t limit = -std::numeric_limits<int_t>::max();
	if (value[0] == u'-' || value[0] == u'+')
	{
		negative = value[0] == u'-';
		if (negative)
			limit = std::numeric_limits<int_t>::min();
		if (++index == value.size())
			return defaultValue;
	}

	int_t result = 0;
	int_t multiplyLimit = limit / 10;
	for (; index < value.size(); ++index)
	{
		char_t character = value[index];
		if (character < u'0' || character > u'9')
			return defaultValue;
		int_t digit = character - u'0';
		if (result < multiplyLimit)
			return defaultValue;
		result *= 10;
		if (result < limit + digit)
			return defaultValue;
		result -= digit;
	}

	return negative ? result : -result;
}

MultiplayerScreen::ServerAddress MultiplayerScreen::parseServerAddress(const jstring &text)
{
	jstring address = trim(text);
	std::vector<jstring> parts = splitOnColon(address);
	if (!address.empty() && address.front() == u'[')
	{
		size_t bracket = address.find(u']');
		if (bracket != jstring::npos && bracket > 0)
		{
			jstring host = address.substr(1, bracket - 1);
			jstring suffix = trim(address.substr(bracket + 1));
			if (!suffix.empty() && suffix.front() == u':')
				parts = {host, suffix.substr(1)};
			else
				parts = {host};
		}
	}

	if (parts.size() > 2)
		parts = {address};

	return {
		String::toUTF8(parts.at(0)),
		parts.size() > 1 ? parseIntWithDefault(parts[1], 25565) : 25565
	};
}
