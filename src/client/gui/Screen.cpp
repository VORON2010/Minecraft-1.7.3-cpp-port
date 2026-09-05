#include "client/gui/Screen.h"

#include "client/Minecraft.h"
#include "client/renderer/Tesselator.h"

#include "OpenGL.h"

#include "lwjgl/Keyboard.h"
#include "lwjgl/Mouse.h"
#include "SDL.h"

#include <iostream>

Screen::Screen(Minecraft &minecraft) : minecraft(minecraft), font(*minecraft.font)
{

}

void Screen::render(int_t xm, int_t ym, float a)
{
	for (auto &button : buttons)
		button->render(minecraft, xm, ym);
}

void Screen::keyPressed(char_t eventCharacter, int_t eventKey)
{
	if (eventKey == lwjgl::Keyboard::KEY_ESCAPE)
	{
		minecraft.setScreen(nullptr);
		minecraft.grabMouse();
	}
}

jstring Screen::getClipboard()
{
	char *text = SDL_GetClipboardText();
	if (text == nullptr)
		return u"";

	jstring result = String::fromUTF8(text);
	SDL_free(text);
	return result;
}

void Screen::setClipboard(const jstring &text)
{
	SDL_SetClipboardText(String::toUTF8(text).c_str());
}

void Screen::selectNextField()
{

}

void Screen::mouseClicked(int_t x, int_t y, int_t buttonNum)
{
	if (buttonNum == 0)
	{
		for (auto &button : buttons)
		{
			if (button->clicked(minecraft, x, y))
			{
				clickedButton = button;
				minecraft.soundEngine.playUI(u"random.click", 1.0f, 1.0f);
				buttonClicked(*button);
			}
		}
	}
}

void Screen::mouseReleased(int_t x, int_t y, int_t buttonNum)
{
	if (clickedButton != nullptr && buttonNum == 0)
	{
		clickedButton->released(x, y);
		clickedButton = nullptr;
	}
}

void Screen::mouseScrolled(int_t x, int_t y, int_t scrollAmount)
{

}

void Screen::buttonClicked(Button &button)
{

}

void Screen::init(int_t width, int_t height)
{
	this->width = width;
	this->height = height;
	buttons.clear();
	init();
}

void Screen::setSize(int_t width, int_t height)
{
	this->width = width;
	this->height = height;
}

void Screen::init()
{

}

void Screen::updateEvents()
{
	while (lwjgl::Mouse::next())
		mouseEvent();
	while (lwjgl::Keyboard::next())
		keyboardEvent();
}

void Screen::mouseEvent()
{
	int_t xm = lwjgl::Mouse::getEventX() * width / minecraft.width;
	int_t ym = height - lwjgl::Mouse::getEventY() * height / minecraft.height - 1;
	int_t scrollAmount = lwjgl::Mouse::getEventDWheel();
	if (scrollAmount != 0)
		mouseScrolled(xm, ym, scrollAmount);

	int_t button = lwjgl::Mouse::getEventButton();
	if (button < 0)
		return;

	if (lwjgl::Mouse::getEventButtonState())
		mouseClicked(xm, ym, button);
	else
		mouseReleased(xm, ym, button);
}

void Screen::keyboardEvent()
{
	if (lwjgl::Keyboard::getEventKeyState())
	{
		if (lwjgl::Keyboard::getEventKey() == lwjgl::Keyboard::KEY_F11)
		{
			minecraft.toggleFullscreen();
			return;
		}
		keyPressed(lwjgl::Keyboard::getEventCharacter(), lwjgl::Keyboard::getEventKey());
	}
}

void Screen::tick()
{

}

void Screen::removed()
{

}

void Screen::renderBackground()
{
	renderBackground(0);
}

void Screen::renderBackground(int_t vo)
{
	if (minecraft.level != nullptr)
		fillGradient(0, 0, width, height, 0xC0101010, 0xD0101010);
	else
		renderDirtBackground(vo);
}

void Screen::renderDirtBackground(int_t vo)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);

	Tesselator &t = Tesselator::instance;

	glBindTexture(GL_TEXTURE_2D, minecraft.textures.loadTexture(u"/gui/background.png"));
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	float s = 32.0f;
	t.begin();
	t.color(0x404040);
	t.vertexUV(0.0, height, 0.0, 0.0, (height / s + vo));
	t.vertexUV(width, height, 0.0, (width / s), (height / s + vo));
	t.vertexUV(width, 0.0, 0.0, (width / s), (0 + vo));
	t.vertexUV(0.0, 0.0, 0.0, 0.0, (0 + vo));
	t.end();
}

bool Screen::isPauseScreen()
{
	return true;
}

void Screen::confirmResult(bool result, int_t id)
{

}

