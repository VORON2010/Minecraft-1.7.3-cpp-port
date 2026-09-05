#include "client/gui/RenameWorldScreen.h"

#include "client/Minecraft.h"
#include "client/locale/Language.h"

#include "client/gui/GuiTextField.h"
#include "client/gui/SelectWorldScreen.h"

#include "lwjgl/Keyboard.h"

namespace
{

bool isWhitespace(char_t c)
{
	return c == u' ' || c == u'\n' || c == u'\r' || c == u'\t' || c == u'\f';
}

jstring trim(const jstring &text)
{
	size_t start = 0;
	while (start < text.size() && isWhitespace(text[start]))
		start++;

	size_t end = text.size();
	while (end > start && isWhitespace(text[end - 1]))
		end--;

	return text.substr(start, end - start);
}

}

RenameWorldScreen::RenameWorldScreen(Minecraft &minecraft, std::shared_ptr<Screen> lastScreen, const jstring &folderName, const jstring &currentName)
	: Screen(minecraft), lastScreen(lastScreen), folderName(folderName), currentName(currentName)
{

}

void RenameWorldScreen::init()
{
	Language &language = Language::getInstance();
	lwjgl::Keyboard::enableRepeatEvents(true);
	buttons.push_back(Util::make_shared<Button>(0, width / 2 - 100, height / 4 + 108, language.getElement(u"selectWorld.renameButton")));
	buttons.push_back(Util::make_shared<Button>(1, width / 2 - 100, height / 4 + 132, language.getElement(u"gui.cancel")));
	nameField = Util::make_shared<GuiTextField>(*this, font, width / 2 - 100, 60, 200, 20, currentName);
	nameField->isFocused = true;
	nameField->setMaxStringLength(32);
}

void RenameWorldScreen::tick()
{
	if (nameField != nullptr)
		nameField->updateCursorCounter();
}

void RenameWorldScreen::removed()
{
	lwjgl::Keyboard::enableRepeatEvents(false);
}

void RenameWorldScreen::buttonClicked(Button &button)
{
	if (!button.active)
		return;

	if (button.id == 0)
		Level::renameLevel(*minecraft.getWorkingDirectory(), folderName, trim(nameField->getText()));

	minecraft.setScreen(Util::make_shared<SelectWorldScreen>(minecraft, lastScreen));
}

void RenameWorldScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
	if (nameField != nullptr)
		nameField->textboxKeyTyped(eventCharacter, eventKey);

	updateRenameButton();

	
	if ((eventCharacter == u'\r' || eventKey == lwjgl::Keyboard::KEY_RETURN || eventKey == lwjgl::Keyboard::KEY_NUMPADENTER) && !buttons.empty())
		buttonClicked(*buttons[0]);
}

void RenameWorldScreen::mouseClicked(int_t x, int_t y, int_t buttonNum)
{
	Screen::mouseClicked(x, y, buttonNum);
	if (nameField != nullptr)
		nameField->mouseClicked(x, y, buttonNum);
}

void RenameWorldScreen::render(int_t xm, int_t ym, float a)
{
	Language &language = Language::getInstance();
	renderBackground();
	drawCenteredString(font, language.getElement(u"selectWorld.renameTitle"), width / 2, height / 4 - 40, 0xFFFFFF);
	drawString(font, language.getElement(u"selectWorld.enterName"), width / 2 - 100, 47, 0xA0A0A0);
	if (nameField != nullptr)
		nameField->drawTextBox();
	Screen::render(xm, ym, a);
}

void RenameWorldScreen::updateRenameButton()
{
	if (!buttons.empty())
		buttons[0]->active = !trim(nameField->getText()).empty();
}
