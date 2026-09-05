#pragma once

#include <memory>

#include "client/gui/Screen.h"

class SignTileEntity;

class EditSignScreen : public Screen
{
private:
	std::shared_ptr<SignTileEntity> sign;
	int_t updateCounter = 0;
	int_t editLine = 0;

public:
	EditSignScreen(Minecraft &minecraft, std::shared_ptr<SignTileEntity> sign);

	void init() override;
	void removed() override;
	void tick() override;
	void render(int_t xm, int_t ym, float a) override;

protected:
	void keyPressed(char_t eventCharacter, int_t eventKey) override;
	void buttonClicked(Button &button) override;
};