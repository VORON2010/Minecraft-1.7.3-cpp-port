#include "client/gui/CreateWorldScreen.h"

#include <limits>
#include <stdexcept>

#include "client/Minecraft.h"
#include "client/locale/Language.h"

#include "client/gamemode/SurvivalMode.h"
#include "client/gamemode/CreativeMode.h"
#include "client/gui/GuiTextField.h"
#include "client/gui/SelectWorldScreen.h"

#include "java/File.h"
#include "java/Number.h"
#include "java/Random.h"
#include "world/level/Level.h"

#include "lwjgl/Keyboard.h"

namespace
{


bool isWhitespace(char_t c)
{
	return static_cast<uchar_t>(c) <= u' ';
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

bool isReservedFolderChar(char_t c)
{
	switch (c)
	{
		case u'/':
		case u'\n':
		case u'\r':
		case u'\t':
		case 0:
		case u'\f':
		case u'`':
		case u'?':
		case u'*':
		case u'\\':
		case u'<':
		case u'>':
		case u'|':
		case u'"':
		case u':':
			return true;
		default:
			return false;
	}
}

jstring sanitizeFolderName(const jstring &text)
{
	jstring folderName = trim(text);
	for (auto &c : folderName)
	{
		if (isReservedFolderChar(static_cast<char_t>(c)))
			c = u'_';
	}
	if (folderName.empty())
		folderName = u"World";
	return folderName;
}


jstring generateUnusedFolderName(File &workingDirectory, const jstring &baseName)
{
	jstring folderName = baseName;
	while (Level::getDataTagFor(workingDirectory, folderName) != nullptr)
		folderName += u"-";
	return folderName;
}


long_t hashSeedString(const jstring &text)
{
	uint_t hash = 0;
	for (char_t c : text)
		hash = 31u * hash + static_cast<uint_t>(static_cast<uchar_t>(c));
	return static_cast<long_t>(Java::intFromBits(hash));
}


bool parseJavaLong(const jstring &text, long_t &value)
{
	if (text.empty())
		return false;
	size_t i = 0;
	bool negative = false;
	if (text[0] == u'-' || text[0] == u'+')
	{
		negative = text[0] == u'-';
		i = 1;
		if (text.size() == 1)
			return false;
	}
	
	long_t limit = negative ? std::numeric_limits<long_t>::min() : -std::numeric_limits<long_t>::max();
	long_t result = 0;
	for (; i < text.size(); ++i)
	{
		char_t c = text[i];
		if (c < u'0' || c > u'9')
			return false;
		int_t digit = c - u'0';
		if (result < limit / 10)
			return false;
		result *= 10;
		if (result < limit + digit)
			return false;
		result -= digit;
	}
	value = negative ? result : -result;
	return true;
}

}

CreateWorldScreen::CreateWorldScreen(Minecraft &minecraft, std::shared_ptr<Screen> lastScreen) : Screen(minecraft), lastScreen(lastScreen)
{

}

void CreateWorldScreen::init()
{
	Language &language = Language::getInstance();
	lwjgl::Keyboard::enableRepeatEvents(true);
	buttons.clear();
	buttons.push_back(Util::make_shared<Button>(0, width / 2 - 155, height - 28, 150, 20, language.getElement(u"selectWorld.create")));
	buttons.push_back(Util::make_shared<Button>(1, width / 2 + 5, height - 28, 150, 20, language.getElement(u"gui.cancel")));
	gameModeButton = Util::make_shared<Button>(2, width / 2 - 75, 100, 150, 20, language.getElement(u"selectWorld.gameMode"));
	moreOptionsButton = Util::make_shared<Button>(3, width / 2 - 75, 172, 150, 20, language.getElement(u"selectWorld.moreWorldOptions"));
	structuresButton = Util::make_shared<Button>(4, width / 2 - 155, 100, 150, 20, language.getElement(u"selectWorld.mapFeatures"));
	structuresButton->visible = false;
	mapTypeButton = Util::make_shared<Button>(5, width / 2 + 5, 100, 150, 20, language.getElement(u"selectWorld.mapType"));
	mapTypeButton->visible = false;
	mapTypeButton->active = false;
	buttons.push_back(gameModeButton);
	buttons.push_back(moreOptionsButton);
	buttons.push_back(structuresButton);
	buttons.push_back(mapTypeButton);
	worldNameField = Util::make_shared<GuiTextField>(*this, font, width / 2 - 100, 60, 200, 20, language.getElement(u"selectWorld.newWorld"));
	worldNameField->isFocused = true;
	worldNameField->setMaxStringLength(32);
	seedField = Util::make_shared<GuiTextField>(*this, font, width / 2 - 100, 60, 200, 20, u"");
	updateFolderName();
	updateSettingsLabels();
	updateCreateButton();
}

void CreateWorldScreen::updateSettingsLabels()
{
	Language &language = Language::getInstance();
	gameModeButton->msg = language.getElement(u"selectWorld.gameMode") + u" " + language.getElement(u"selectWorld.gameMode." + gameModeName);
	gameModeLine1 = language.getElement(u"selectWorld.gameMode." + gameModeName + u".line1");
	gameModeLine2 = language.getElement(u"selectWorld.gameMode." + gameModeName + u".line2");
	structuresButton->msg = language.getElement(u"selectWorld.mapFeatures") + u" " + language.getElement(structures ? u"options.on" : u"options.off");
	mapTypeButton->msg = language.getElement(u"selectWorld.mapType") + u" " + language.getElement(u"selectWorld.mapType.normal");
}

void CreateWorldScreen::tick()
{
	if (worldNameField != nullptr)
		worldNameField->updateCursorCounter();
	if (seedField != nullptr)
		seedField->updateCursorCounter();
}

void CreateWorldScreen::removed()
{
	lwjgl::Keyboard::enableRepeatEvents(false);
}

void CreateWorldScreen::selectNextField()
{
	if (worldNameField == nullptr || seedField == nullptr)
		return;

	if (worldNameField->isFocused)
	{
		worldNameField->setFocused(false);
		seedField->setFocused(true);
	}
	else
	{
		worldNameField->setFocused(true);
		seedField->setFocused(false);
	}
}

void CreateWorldScreen::buttonClicked(Button &button)
{
	if (!button.active)
		return;

	if (button.id == 1)
	{
		minecraft.setScreen(Util::make_shared<SelectWorldScreen>(minecraft, lastScreen));
		return;
	}

	if (button.id == 0)
	{
		if (createClicked)
			return;
		createClicked = true;
		minecraft.setScreen(nullptr);
		Level::nextCreateSettings.gameType = gameModeName == u"creative" ? 1 : 0;
		Level::nextCreateSettings.mapFeatures = structures;
		Level::nextCreateSettings.hardcore = hardcore;
		if (gameModeName == u"creative")
			minecraft.gameMode = Util::make_shared<CreativeMode>(minecraft);
		else
			minecraft.gameMode = Util::make_shared<SurvivalMode>(minecraft);
		minecraft.selectLevel(folderName, worldNameField->getText(), parseSeed());
		minecraft.setScreen(nullptr);
	}
	else if (button.id == 3)
	{
		Language &language = Language::getInstance();
		moreOptionsOpen = !moreOptionsOpen;
		gameModeButton->visible = !moreOptionsOpen;
		structuresButton->visible = moreOptionsOpen;
		mapTypeButton->visible = moreOptionsOpen;
		moreOptionsButton->msg = language.getElement(moreOptionsOpen ? u"gui.done" : u"selectWorld.moreWorldOptions");
	}
	else if (button.id == 2)
	{
		// survival -> hardcore -> creative
		if (gameModeName == u"survival")
		{
			gameModeName = u"hardcore";
			hardcore = true;
		}
		else if (gameModeName == u"hardcore")
		{
			gameModeName = u"creative";
			hardcore = false;
		}
		else
		{
			gameModeName = u"survival";
			hardcore = false;
		}
		updateSettingsLabels();
	}
	else if (button.id == 4)
	{
		structures = !structures;
		updateSettingsLabels();
	}
}

void CreateWorldScreen::keyPressed(char_t eventCharacter, int_t eventKey)
{
	if (worldNameField != nullptr && worldNameField->isFocused && !moreOptionsOpen)
		worldNameField->textboxKeyTyped(eventCharacter, eventKey);
	else if (seedField != nullptr && seedField->isFocused && moreOptionsOpen)
		seedField->textboxKeyTyped(eventCharacter, eventKey);

	
	if ((eventCharacter == u'\r' || eventKey == lwjgl::Keyboard::KEY_RETURN || eventKey == lwjgl::Keyboard::KEY_NUMPADENTER) && !buttons.empty())
		buttonClicked(*buttons[0]);

	updateFolderName();
	updateCreateButton();
}

void CreateWorldScreen::mouseClicked(int_t x, int_t y, int_t buttonNum)
{
	Screen::mouseClicked(x, y, buttonNum);
	if (!moreOptionsOpen && worldNameField != nullptr)
		worldNameField->mouseClicked(x, y, buttonNum);
	else if (moreOptionsOpen && seedField != nullptr)
		seedField->mouseClicked(x, y, buttonNum);
}

void CreateWorldScreen::render(int_t xm, int_t ym, float a)
{
	Language &language = Language::getInstance();
	renderBackground();
	drawCenteredString(font, language.getElement(u"selectWorld.create"), width / 2, 20, 0xFFFFFF);
	if (!moreOptionsOpen)
	{
		drawString(font, language.getElement(u"selectWorld.enterName"), width / 2 - 100, 47, 0xA0A0A0);
		drawString(font, language.getElement(u"selectWorld.resultFolder") + u" " + folderName, width / 2 - 100, 85, 0xA0A0A0);
		if (worldNameField != nullptr)
			worldNameField->drawTextBox();
		drawString(font, gameModeLine1, width / 2 - 100, 122, 0xA0A0A0);
		drawString(font, gameModeLine2, width / 2 - 100, 134, 0xA0A0A0);
	}
	else
	{
		drawString(font, language.getElement(u"selectWorld.enterSeed"), width / 2 - 100, 47, 0xA0A0A0);
		drawString(font, language.getElement(u"selectWorld.seedInfo"), width / 2 - 100, 85, 0xA0A0A0);
		drawString(font, language.getElement(u"selectWorld.mapFeatures.info"), width / 2 - 150, 122, 0xA0A0A0);
		if (seedField != nullptr)
			seedField->drawTextBox();
	}
	Screen::render(xm, ym, a);
}

void CreateWorldScreen::updateFolderName()
{
	folderName = generateUnusedFolderName(*minecraft.getWorkingDirectory(), sanitizeFolderName(worldNameField->getText()));
}

void CreateWorldScreen::updateCreateButton()
{
	if (!buttons.empty())
		buttons[0]->active = !worldNameField->getText().empty();
}

long_t CreateWorldScreen::parseSeed() const
{
	Random random;
	long_t seed = random.nextLong();
	const jstring &seedText = seedField->getText();
	if (seedText.empty())
		return seed;

	long_t parsed = 0;
	if (!parseJavaLong(seedText, parsed))
		return hashSeedString(seedText);
	return parsed != 0 ? parsed : seed;
}
