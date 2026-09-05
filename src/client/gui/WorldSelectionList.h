#pragma once

#include "client/gui/GuiSlot.h"

class SelectWorldScreen;

class WorldSelectionList : public GuiSlot
{
private:
	SelectWorldScreen &screen;

public:
	WorldSelectionList(Minecraft &minecraft, SelectWorldScreen &screen);

protected:
	int_t getSize() override;
	void elementClicked(int_t index, bool doubleClick) override;
	bool isSelected(int_t index) override;
	int_t getContentHeight() override;
	void drawBackground() override;
	void drawSlot(int_t index, int_t x, int_t y, int_t height, Tesselator &t) override;
};
