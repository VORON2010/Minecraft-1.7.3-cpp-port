#pragma once

#include "client/gui/GuiComponent.h"
#include "client/gui/Font.h"

#include "java/Type.h"
#include "java/String.h"

class Screen;

class GuiTextField : public GuiComponent
{
private:
	Screen &parentScreen;
	Font &font;
	int_t xPos;
	int_t yPos;
	int_t width;
	int_t height;
	jstring text;
	int_t maxStringLength = 0;
	int_t cursorCounter = 0;

public:
	bool isFocused = false;
	bool isEnabled = true;

	GuiTextField(Screen &parentScreen, Font &font, int_t x, int_t y, int_t width, int_t height, const jstring &text);

	void setText(const jstring &text);
	const jstring &getText() const;

	void updateCursorCounter();
	void textboxKeyTyped(char_t eventCharacter, int_t eventKey);
	void mouseClicked(int_t x, int_t y, int_t buttonNum);
	void setFocused(bool focused);
	void drawTextBox();
	void setMaxStringLength(int_t maxStringLength);
};
