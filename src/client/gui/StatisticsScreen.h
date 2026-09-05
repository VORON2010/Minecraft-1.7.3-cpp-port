#pragma once

#include <memory>

#include "client/gui/Screen.h"

class GuiSlot;
class GeneralStatisticsList;
class ItemStatisticsList;
class BlockStatisticsList;
class StatFileWriter;

class StatisticsScreen : public Screen
{
	friend class GeneralStatisticsList;
	friend class ItemStatisticsList;
	friend class BlockStatisticsList;
	friend class StatisticsList;

private:
	std::shared_ptr<Screen> lastScreen;
	StatFileWriter &stats;
	std::unique_ptr<GeneralStatisticsList> generalList;
	std::unique_ptr<ItemStatisticsList> itemList;
	std::unique_ptr<BlockStatisticsList> blockList;
	GuiSlot *activeList = nullptr;
	jstring title = u"Select world";

	void addButtons();
	void drawStatsTexture(int_t x, int_t y, int_t u, int_t v);
	void drawStatsItem(int_t x, int_t y, int_t itemId);
	void drawTooltip(const jstring &text, int_t x, int_t y);
	jstring getItemName(int_t itemId) const;

protected:
	void buttonClicked(Button &button) override;

public:
	StatisticsScreen(Minecraft &minecraft, std::shared_ptr<Screen> lastScreen, StatFileWriter &stats);
	~StatisticsScreen() override;

	void init() override;
	void render(int_t mouseX, int_t mouseY, float partialTick) override;
};
