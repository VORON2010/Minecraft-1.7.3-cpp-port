#pragma once

#include <vector>

#include "client/gui/Screen.h"

// 1.0 CreditsScreen: the End poem (title/win.txt) and credits scrolling after leaving the End
class WinScreen : public Screen
{
	int_t ticks = 0;
	std::vector<jstring> lines;
	std::vector<int_t> lineHeights;
	int_t totalHeight = 0;
	bool loaded = false;
	float speed = 0.5f;

public:
	explicit WinScreen(Minecraft &minecraft);

	void tick() override;
	void init() override;
	void render(int_t xm, int_t ym, float a) override;
	bool isPauseScreen() override;

protected:
	void keyPressed(char_t eventCharacter, int_t eventKey) override;

private:
	void respawn();
	void renderBg(float a);
	void readLines(const jstring &path, bool tabs, Random *obfuscate);
};
