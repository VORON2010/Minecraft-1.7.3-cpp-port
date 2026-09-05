#include "client/gui/SleepScreen.h"

#include <memory>

#include "client/Minecraft.h"
#include "client/locale/Language.h"
#include "client/player/MultiplayerLocalPlayer.h"
#include "lwjgl/Keyboard.h"
#include "network/NetClientHandler.h"
#include "network/PacketCore.h"

namespace
{

jstring trim(jstring text)
{
	while (!text.empty() && text.front() <= u' ')
		text.erase(text.begin());
	while (!text.empty() && text.back() <= u' ')
		text.pop_back();
	return text;
}

}

SleepScreen::SleepScreen(Minecraft &minecraft) : ChatScreen(minecraft)
{
}

void SleepScreen::init()
{
	lwjgl::Keyboard::enableRepeatEvents(true);
	buttons.push_back(Util::make_shared<Button>(1, width / 2 - 100, height - 40,
		Language::getInstance().getElement(u"multiplayer.stopSleeping")));
}

void SleepScreen::removed()
{
	lwjgl::Keyboard::enableRepeatEvents(false);
}

void SleepScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
	if (eventKey == lwjgl::Keyboard::KEY_ESCAPE)
	{
		stopSleeping();
	}
	else if (eventKey == lwjgl::Keyboard::KEY_RETURN)
	{
		jstring text = trim(message);
		if (!text.empty() && minecraft.player != nullptr)
			minecraft.player->sendChatMessage(text);
		message.clear();
	}
	else
	{
		ChatScreen::keyPressed(eventCharacter, eventKey);
	}
}

void SleepScreen::buttonClicked(Button &button)
{
	if (button.id == 1)
		stopSleeping();
	else
		Screen::buttonClicked(button);
}

void SleepScreen::stopSleeping()
{
	MultiplayerLocalPlayer *multiplayerPlayer =
		dynamic_cast<MultiplayerLocalPlayer *>(minecraft.player.get());
	if (multiplayerPlayer != nullptr)
	{
		multiplayerPlayer->sendQueue.addToSendQueue(
			std::make_unique<Packet19EntityAction>(*minecraft.player, 3));
	}
}

void SleepScreen::render(int_t xm, int_t ym, float a)
{
	ChatScreen::render(xm, ym, a);
	Screen::render(xm, ym, a);
}
