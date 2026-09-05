#pragma once

#include <vector>

#include "client/gui/Screen.h"
#include "world/level/Level.h"

class WorldSelectionList;

class SelectWorldScreen : public Screen
{
protected:
	std::shared_ptr<Screen> lastScreen;
	std::shared_ptr<WorldSelectionList> worldList;
	std::vector<Level::Summary> worlds;
	std::shared_ptr<Button> selectButton;
	std::shared_ptr<Button> renameButton;
	std::shared_ptr<Button> deleteButton;
	jstring title = u"Select World";
	int_t selectedWorld = -1;
	bool done = false;

public:
	SelectWorldScreen(Minecraft &minecraft, std::shared_ptr<Screen> lastScreen);

	void init() override;
	void confirmResult(bool result, int_t id) override;

protected:
	void buttonClicked(Button &button) override;
	void loadWorlds();

public:
	void mouseScrolled(int_t x, int_t y, int_t scrollAmount) override;
	void render(int_t xm, int_t ym, float a) override;
	const std::vector<Level::Summary> &getWorlds() const;
	const Level::Summary *getSummary(int_t index) const;
	int_t getSelectedWorld() const;
	void setSelectedWorld(int_t index);
	void updateSelectionButtons();
	void selectWorld(int_t index);
};
