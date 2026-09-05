#pragma once

#include "client/gui/Screen.h"

class ConflictWarningScreen : public Screen
{
public:
	ConflictWarningScreen(Minecraft &minecraft);

	void init() override;

protected:
	void buttonClicked(Button &button) override;

public:
	void render(int_t xm, int_t ym, float a) override;
};
