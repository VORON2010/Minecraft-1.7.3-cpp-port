#pragma once

#include <memory>

#include "client/gui/Screen.h"

class NetClientHandler;

class DownloadTerrainScreen : public Screen
{
private:
	std::shared_ptr<NetClientHandler> netHandler;
	int_t updateCounter = 0;

public:
	DownloadTerrainScreen(Minecraft &minecraft, std::shared_ptr<NetClientHandler> netHandler);

protected:
	void keyPressed(char_t eventCharacter, int_t eventKey) override;

public:
	void init() override;
	void tick() override;

protected:
	void buttonClicked(Button &button) override;

public:
	void render(int_t xm, int_t ym, float a) override;
};
