#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "client/gui/Screen.h"

class NetClientHandler;

class ConnectingScreen : public Screen
{
private:
	struct State;
	std::shared_ptr<State> state;

public:
	ConnectingScreen(Minecraft &minecraft, const std::string &hostName, int_t port);

	void tick() override;

protected:
	void keyPressed(char_t eventCharacter, int_t eventKey) override;

public:
	void init() override;

protected:
	void buttonClicked(Button &button) override;

public:
	void render(int_t xm, int_t ym, float a) override;
};
