#include "client/gui/GuiTextField.h"

#include "client/gui/Screen.h"
#include "SharedConstants.h"
#include "lwjgl/Keyboard.h"



static bool isPasteShortcutDown()
{
	return lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_LCONTROL)
		|| lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_RCONTROL)
		|| lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_LMETA)
		|| lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_RMETA);
}

GuiTextField::GuiTextField(Screen &parentScreen, Font &font, int_t x, int_t y, int_t width, int_t height, const jstring &text)
	: parentScreen(parentScreen), font(font), xPos(x), yPos(y), width(width), height(height)
{
	setText(text);
}

void GuiTextField::setText(const jstring &text)
{
	this->text = text;
}

const jstring &GuiTextField::getText() const
{
	return text;
}

void GuiTextField::updateCursorCounter()
{
	cursorCounter++;
}

void GuiTextField::textboxKeyTyped(char_t eventCharacter, int_t eventKey)
{
	if (!isEnabled || !isFocused)
		return;

	if (eventCharacter == u'\t' || eventKey == lwjgl::Keyboard::KEY_TAB)
		parentScreen.selectNextField();

	if (eventCharacter == 0x16 || (eventKey == lwjgl::Keyboard::KEY_V && isPasteShortcutDown()))
	{
		
		jstring clipboard = parentScreen.getClipboard();
		int_t room = 32 - static_cast<int_t>(text.length());
		if (room > static_cast<int_t>(clipboard.length()))
			room = static_cast<int_t>(clipboard.length());
		if (room > 0)
			text += clipboard.substr(0, room);
	}

	if (eventKey == lwjgl::Keyboard::KEY_BACK && !text.empty())
		text.pop_back();

	if (SharedConstants::acceptableLetters.find(eventCharacter) != jstring::npos
		&& (static_cast<int_t>(text.length()) < maxStringLength || maxStringLength == 0))
		text += eventCharacter;
}

void GuiTextField::mouseClicked(int_t x, int_t y, int_t buttonNum)
{
	bool inside = isEnabled && x >= xPos && x < xPos + width && y >= yPos && y < yPos + height;
	setFocused(inside);
}

void GuiTextField::setFocused(bool focused)
{
	if (focused && !isFocused)
		cursorCounter = 0;
	isFocused = focused;
}

void GuiTextField::drawTextBox()
{
	fill(xPos - 1, yPos - 1, xPos + width + 1, yPos + height + 1, 0xFFA0A0A0);
	fill(xPos, yPos, xPos + width, yPos + height, 0xFF000000);
	if (isEnabled)
	{
		bool cursor = isFocused && cursorCounter / 6 % 2 == 0;
		drawString(font, text + (cursor ? u"_" : u""), xPos + 4, yPos + (height - 8) / 2, 0xE0E0E0);
	}
	else
	{
		drawString(font, text, xPos + 4, yPos + (height - 8) / 2, 0x707070);
	}
}

void GuiTextField::setMaxStringLength(int_t maxStringLength)
{
	this->maxStringLength = maxStringLength;
}
