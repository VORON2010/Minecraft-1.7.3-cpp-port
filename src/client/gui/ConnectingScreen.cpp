#include "client/gui/ConnectingScreen.h"

#include <atomic>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>

#include "client/Minecraft.h"
#include "client/User.h"
#include "client/gui/ConnectFailedScreen.h"
#include "client/locale/Language.h"
#include "client/title/TitleScreen.h"
#include "network/NetClientHandler.h"
#include "network/PacketCore.h"

struct ConnectingScreen::State
{
	std::mutex mutex;
	std::shared_ptr<NetClientHandler> clientHandler;
	std::atomic<bool> cancelled{false};
	bool failureReady = false;
	jstring failureDetail;
};

namespace
{
	const char *const UNKNOWN_HOST_PREFIX = "Unable to resolve ";
	const char *const CONNECT_FAILURE_PREFIX = "Unable to connect to ";

	jstring translate(const jstring &key)
	{
		return Language::getInstance().getElement(key);
	}

	std::string connectFailureDetail(const std::string &message)
	{
		const size_t separator = message.rfind(": ");
		return separator == std::string::npos ? message : message.substr(separator + 2);
	}
}

ConnectingScreen::ConnectingScreen(Minecraft &minecraft, const std::string &hostName,
	int_t port)
	: Screen(minecraft), state(std::make_shared<State>())
{
	std::cout << "Connecting to " << hostName << ", " << port << std::endl;
	minecraft.setLevel(nullptr);

	std::shared_ptr<State> connectionState = state;
	minecraft.connectionThreads.emplace_back([connectionState, &minecraft, hostName, port]()
	{
		try
		{
			if (port < 0 || port > 65535)
				throw std::invalid_argument("java.lang.IllegalArgumentException: port out of range:" +
					std::to_string(port));
			std::shared_ptr<NetClientHandler> clientHandler =
				NetClientHandler::connect(minecraft, hostName, static_cast<std::uint16_t>(port));
			{
				std::lock_guard<std::mutex> lock(connectionState->mutex);
				connectionState->clientHandler = clientHandler;
			}

			if (connectionState->cancelled.load())
			{
				clientHandler->disconnect();
				return;
			}

			clientHandler->addToSendQueue(
				std::make_unique<Packet2Handshake>(minecraft.user->name));
		}
		catch (const std::exception &exception)
		{
			if (connectionState->cancelled.load())
				return;

			const std::string detail = exception.what();
			jstring failureDetail;
			if (detail.compare(0, std::char_traits<char>::length(UNKNOWN_HOST_PREFIX),
				UNKNOWN_HOST_PREFIX) == 0)
			{
				failureDetail = u"Unknown host '" + String::fromUTF8(hostName) + u"'";
			}
			else if (detail.compare(0, std::char_traits<char>::length(CONNECT_FAILURE_PREFIX),
				CONNECT_FAILURE_PREFIX) == 0)
			{
				failureDetail = String::fromUTF8(connectFailureDetail(detail));
			}
			else
			{
				std::cerr << detail << std::endl;
				failureDetail = String::fromUTF8(detail);
			}

			std::lock_guard<std::mutex> lock(connectionState->mutex);
			if (!connectionState->cancelled.load())
			{
				connectionState->failureReady = true;
				connectionState->failureDetail = std::move(failureDetail);
			}
		}
	});
}

void ConnectingScreen::tick()
{
	std::shared_ptr<NetClientHandler> clientHandler;
	bool failureReady = false;
	jstring failureDetail;
	{
		std::lock_guard<std::mutex> lock(state->mutex);
		clientHandler = state->clientHandler;
		failureReady = state->failureReady;
		if (failureReady)
		{
			failureDetail = state->failureDetail;
			state->failureReady = false;
		}
	}
	if (state->cancelled.load())
		return;
	if (failureReady)
	{
		minecraft.setScreen(Util::make_shared<ConnectFailedScreen>(minecraft,
			u"connect.failed", u"disconnect.genericReason", failureDetail));
		return;
	}
	if (clientHandler != nullptr)
	{
		minecraft.connection = clientHandler;
		clientHandler->processReadPackets();
	}
}

void ConnectingScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
}

void ConnectingScreen::init()
{
	buttons.clear();
	buttons.push_back(Util::make_shared<Button>(0, width / 2 - 100, height / 4 + 120 + 12,
		translate(u"gui.cancel")));
}

void ConnectingScreen::buttonClicked(Button &button)
{
	if (button.id == 0)
	{
		state->cancelled.store(true);

		std::shared_ptr<NetClientHandler> clientHandler;
		{
			std::lock_guard<std::mutex> lock(state->mutex);
			clientHandler = state->clientHandler;
		}
		if (clientHandler != nullptr)
			clientHandler->disconnect();

		minecraft.setScreen(Util::make_shared<TitleScreen>(minecraft));
	}
}

void ConnectingScreen::render(int_t xm, int_t ym, float a)
{
	renderBackground();

	std::shared_ptr<NetClientHandler> clientHandler;
	{
		std::lock_guard<std::mutex> lock(state->mutex);
		clientHandler = state->clientHandler;
	}
	if (clientHandler == nullptr)
	{
		drawCenteredString(font, translate(u"connect.connecting"), width / 2,
			height / 2 - 50, 0xFFFFFF);
		drawCenteredString(font, u"", width / 2, height / 2 - 10, 0xFFFFFF);
	}
	else
	{
		drawCenteredString(font, translate(u"connect.authorizing"), width / 2,
			height / 2 - 50, 0xFFFFFF);
		drawCenteredString(font, clientHandler->field_1209_a, width / 2,
			height / 2 - 10, 0xFFFFFF);
	}

	Screen::render(xm, ym, a);
}
