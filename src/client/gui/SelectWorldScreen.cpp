#include "client/gui/SelectWorldScreen.h"

#include "client/Minecraft.h"
#include "client/locale/Language.h"

#include "client/gamemode/SurvivalMode.h"
#include "client/gui/ConfirmScreen.h"
#include "client/gui/CreateWorldScreen.h"
#include "client/gui/RenameWorldScreen.h"
#include "client/gui/WorldSelectionList.h"

SelectWorldScreen::SelectWorldScreen(Minecraft &minecraft, std::shared_ptr<Screen> lastScreen) : Screen(minecraft), lastScreen(lastScreen)
{

}

void SelectWorldScreen::init()
{
	Language &language = Language::getInstance();
	title = language.getElement(u"selectWorld.title");
	loadWorlds();
	worldList = Util::make_shared<WorldSelectionList>(minecraft, *this);
	buttons.push_back(selectButton = Util::make_shared<Button>(1, width / 2 - 154, height - 52, 150, 20, language.getElement(u"selectWorld.select")));
	buttons.push_back(renameButton = Util::make_shared<Button>(6, width / 2 - 154, height - 28, 70, 20, language.getElement(u"selectWorld.rename")));
	buttons.push_back(deleteButton = Util::make_shared<Button>(2, width / 2 - 74, height - 28, 70, 20, language.getElement(u"selectWorld.delete")));
	buttons.push_back(Util::make_shared<Button>(3, width / 2 + 4, height - 52, 150, 20, language.getElement(u"selectWorld.create")));
	buttons.push_back(Util::make_shared<Button>(0, width / 2 + 4, height - 28, 150, 20, language.getElement(u"gui.cancel")));
	updateSelectionButtons();
}

void SelectWorldScreen::loadWorlds()
{
	worlds = Level::getLevelList(*minecraft.getWorkingDirectory());
	selectedWorld = -1;
}

void SelectWorldScreen::buttonClicked(Button &button)
{
	if (!button.active)
		return;

	Language &language = Language::getInstance();
	const Level::Summary *summary = getSummary(selectedWorld);
	if (button.id == 2 && summary != nullptr)
	{
		jstring question = language.getElement(u"selectWorld.deleteQuestion");
		jstring warning = u"'" + summary->levelName + u"' " + language.getElement(u"selectWorld.deleteWarning");
		minecraft.setScreen(Util::make_shared<ConfirmScreen>(minecraft, minecraft.screen, question, warning, selectedWorld, language.getElement(u"selectWorld.deleteButton"), language.getElement(u"gui.cancel")));
	}
	else if (button.id == 1)
	{
		selectWorld(selectedWorld);
	}
	else if (button.id == 3)
	{
		minecraft.setScreen(Util::make_shared<CreateWorldScreen>(minecraft, lastScreen));
	}
	else if (button.id == 6 && summary != nullptr)
	{
		minecraft.setScreen(Util::make_shared<RenameWorldScreen>(minecraft, lastScreen, summary->folderName, summary->levelName));
	}
	else if (button.id == 0)
	{
		minecraft.setScreen(lastScreen);
	}
}

void SelectWorldScreen::confirmResult(bool result, int_t id)
{
	const Level::Summary *summary = getSummary(id);
	if (result && summary != nullptr)
		Level::deleteLevel(*minecraft.getWorkingDirectory(), summary->folderName);
	minecraft.setScreen(Util::make_shared<SelectWorldScreen>(minecraft, lastScreen));
}

void SelectWorldScreen::mouseScrolled(int_t x, int_t y, int_t scrollAmount)
{
	if (worldList != nullptr)
		worldList->mouseScrolled(scrollAmount);
}

void SelectWorldScreen::render(int_t xm, int_t ym, float a)
{
	if (worldList != nullptr)
		worldList->drawScreen(xm, ym, a);
	else
		renderBackground();

	drawCenteredString(font, title, width / 2, 20, 0xFFFFFF);
	Screen::render(xm, ym, a);
}

const std::vector<Level::Summary> &SelectWorldScreen::getWorlds() const
{
	return worlds;
}

const Level::Summary *SelectWorldScreen::getSummary(int_t index) const
{
	if (index < 0 || index >= static_cast<int_t>(worlds.size()))
		return nullptr;
	return &worlds[index];
}

int_t SelectWorldScreen::getSelectedWorld() const
{
	return selectedWorld;
}

void SelectWorldScreen::setSelectedWorld(int_t index)
{
	selectedWorld = index;
}

void SelectWorldScreen::updateSelectionButtons()
{
	bool hasSelection = getSummary(selectedWorld) != nullptr;
	if (selectButton != nullptr)
		selectButton->active = hasSelection;
	if (renameButton != nullptr)
		renameButton->active = hasSelection;
	if (deleteButton != nullptr)
		deleteButton->active = hasSelection;
}

void SelectWorldScreen::selectWorld(int_t index)
{
	const Level::Summary *summary = getSummary(index);
	if (summary == nullptr || done)
		return;

	done = true;
	minecraft.gameMode = Util::make_shared<SurvivalMode>(minecraft);
	minecraft.selectLevel(summary->folderName);
	minecraft.setScreen(nullptr);
}
