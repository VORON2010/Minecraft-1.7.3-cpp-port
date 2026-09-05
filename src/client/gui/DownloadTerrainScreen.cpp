#include "client/gui/DownloadTerrainScreen.h"

#include <cstring>
#include <memory>
#include <utility>

#include "client/locale/Language.h"
#include "network/NetClientHandler.h"
#include "network/PacketCore.h"

namespace
{
	int_t incrementJavaInt(int_t value)
	{
		uint_t bits;
		std::memcpy(&bits, &value, sizeof(bits));
		++bits;
		std::memcpy(&value, &bits, sizeof(value));
		return value;
	}

	jstring translate(const jstring &key)
	{
		return Language::getInstance().getElement(key);
	}
}

DownloadTerrainScreen::DownloadTerrainScreen(Minecraft &minecraft,
	std::shared_ptr<NetClientHandler> netHandler)
	: Screen(minecraft), netHandler(std::move(netHandler))
{
}

void DownloadTerrainScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
}

void DownloadTerrainScreen::init()
{
	buttons.clear();
}

void DownloadTerrainScreen::tick()
{
	updateCounter = incrementJavaInt(updateCounter);
	if (updateCounter % 20 == 0)
		netHandler->addToSendQueue(std::make_unique<Packet0KeepAlive>());

	if (netHandler != nullptr)
		netHandler->processReadPackets();
}

void DownloadTerrainScreen::buttonClicked(Button &button)
{
}

void DownloadTerrainScreen::render(int_t xm, int_t ym, float a)
{
	renderDirtBackground(0);
	drawCenteredString(font, translate(u"multiplayer.downloadingTerrain"), width / 2,
		height / 2 - 50, 0xFFFFFF);
	Screen::render(xm, ym, a);
}
