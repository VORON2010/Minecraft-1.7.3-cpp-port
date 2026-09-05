#pragma once

#include <memory>
#include <string>

#include "client/gui/GuiTextField.h"
#include "client/gui/Screen.h"

class MultiplayerScreen : public Screen
{
private:
	struct ServerAddress
	{
		std::string host;
		int_t port;
	};

	std::shared_ptr<Screen> parentScreen;
	std::unique_ptr<GuiTextField> serverAddress;

	static jstring trim(const jstring &text);
	static jstring replace(const jstring &text, char_t from, char_t to);
	static int_t parseIntWithDefault(const jstring &text, int_t defaultValue);
	static ServerAddress parseServerAddress(const jstring &text);

	friend int runMultiplayerScreenSmoke();

public:
	MultiplayerScreen(Minecraft &minecraft, std::shared_ptr<Screen> parentScreen);

	void init() override;
	void removed() override;
	void tick() override;
	void render(int_t xm, int_t ym, float a) override;

protected:
	void keyPressed(char_t eventCharacter, int_t eventKey) override;
	void mouseClicked(int_t x, int_t y, int_t buttonNum) override;
	void buttonClicked(Button &button) override;
};
