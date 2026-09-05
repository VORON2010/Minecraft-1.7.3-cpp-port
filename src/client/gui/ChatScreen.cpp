#include "client/gui/ChatScreen.h"

#include "SharedConstants.h"
#include "client/Minecraft.h"

#include "lwjgl/Keyboard.h"

namespace
{

jstring trimWhitespace(jstring text)
{
	while (!text.empty() && (text.front() == u' ' || text.front() == u'\t' || text.front() == u'\n' || text.front() == u'\r'))
		text.erase(text.begin());
	while (!text.empty() && (text.back() == u' ' || text.back() == u'\t' || text.back() == u'\n' || text.back() == u'\r'))
		text.pop_back();
	return text;
}

}

ChatScreen::ChatScreen(Minecraft &minecraft) : Screen(minecraft)
{
}

void ChatScreen::init()
{
	Screen::init();
	lwjgl::Keyboard::enableRepeatEvents(true);
}

void ChatScreen::removed()
{
	lwjgl::Keyboard::enableRepeatEvents(false);
}

void ChatScreen::tick()
{
	Screen::tick();
	frame++;
}

void ChatScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
	if (eventKey == lwjgl::Keyboard::KEY_ESCAPE)
	{
		minecraft.setScreen(nullptr);
		return;
	}
	if (eventKey == lwjgl::Keyboard::KEY_RETURN)
	{
		jstring text = trimWhitespace(message);
		if (!text.empty() && minecraft.player != nullptr)
			minecraft.player->sendChatMessage(text);
		minecraft.setScreen(nullptr);
		return;
	}
	if (eventKey == lwjgl::Keyboard::KEY_BACK)
	{
		if (!message.empty())
			message.pop_back();
		return;
	}
	if (SharedConstants::acceptableLetters.find(eventCharacter) != jstring::npos && message.length() < 100)
		message.push_back(eventCharacter);
}

void ChatScreen::render(int_t xm, int_t ym, float a)
{
	(void)xm;
	(void)ym;
	(void)a;

	fill(2, height - 14, width - 2, height - 2, 0x80000000);
	drawString(font, u"> " + message + ((frame / 6 % 2 == 0) ? u"_" : u""), 4, height - 12, 14737632);
}

