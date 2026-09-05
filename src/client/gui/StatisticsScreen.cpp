#include "client/gui/StatisticsScreen.h"

#include <algorithm>
#include <cstring>
#include <utility>
#include <vector>

#include "client/Lighting.h"
#include "client/Minecraft.h"
#include "client/gui/GuiSlot.h"
#include "client/locale/Language.h"
#include "client/renderer/Tesselator.h"
#include "client/renderer/entity/EntityRenderDispatcher.h"
#include "client/renderer/entity/ItemRenderer.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/level/tile/Tile.h"
#include "world/stats/StatBase.h"
#include "world/stats/StatCollector.h"
#include "world/stats/StatCrafting.h"
#include "world/stats/StatFileWriter.h"
#include "world/stats/StatList.h"

#include "lwjgl/Mouse.h"
#include "OpenGL.h"

namespace
{

jstring trimJava(const jstring &text)
{
	size_t start = 0;
	while (start < text.size() && text[start] <= u' ')
		start++;

	size_t end = text.size();
	while (end > start && text[end - 1] <= u' ')
		end--;

	return text.substr(start, end - start);
}

int_t javaStatComparison(int_t left, int_t right, int_t direction)
{
	uint_t resultBits = (static_cast<uint_t>(left) - static_cast<uint_t>(right)) * static_cast<uint_t>(direction);
	int_t result;
	std::memcpy(&result, &resultBits, sizeof(result));
	return result;
}

}

class GeneralStatisticsList : public GuiSlot
{
private:
	StatisticsScreen &screen;

protected:
	int_t getSize() override
	{
		return static_cast<int_t>(StatList::generalStats.size());
	}

	void elementClicked(int_t, bool) override
	{
	}

	bool isSelected(int_t) override
	{
		return false;
	}

	int_t getContentHeight() override
	{
		return getSize() * 10;
	}

	void drawBackground() override
	{
		screen.renderBackground();
	}

	void drawSlot(int_t index, int_t x, int_t y, int_t, Tesselator &) override
	{
		StatBase *stat = StatList::generalStats[index];
		int_t color = index % 2 == 0 ? 0xFFFFFF : 0x909090;
		screen.drawString(screen.font, stat->statName, x + 2, y + 1, color);
		jstring value = stat->format(screen.stats.getStat(*stat));
		screen.drawString(screen.font, value, x + 215 - screen.font.width(value), y + 1, color);
	}

public:
	GeneralStatisticsList(StatisticsScreen &screen)
		: GuiSlot(screen.minecraft, screen.width, screen.height, 32, screen.height - 64, 10), screen(screen)
	{
		setRenderSelection(false);
	}
};

class StatisticsList : public GuiSlot
{
protected:
	StatisticsScreen &screen;
	std::vector<StatCrafting *> entries;
	int_t pressedColumn = -1;
	int_t sortColumn = -1;
	int_t sortDirection = 0;

	StatisticsList(StatisticsScreen &screen)
		: GuiSlot(screen.minecraft, screen.width, screen.height, 32, screen.height - 64, 20), screen(screen)
	{
		setRenderSelection(false);
		setHeader(true, 20);
	}

	void elementClicked(int_t, bool) override
	{
	}

	bool isSelected(int_t) override
	{
		return false;
	}

	void drawBackground() override
	{
		screen.renderBackground();
	}

	int_t getSize() override
	{
		return static_cast<int_t>(entries.size());
	}

	StatCrafting *getEntry(int_t index)
	{
		return entries[index];
	}

	virtual const char16_t *getColumnName(int_t column) const = 0;
	virtual StatBase *getColumnStat(int_t itemId, int_t column) const = 0;
	virtual void drawColumnIcons(int_t x, int_t y) = 0;

	void drawHeader(int_t x, int_t y, Tesselator &) override
	{
		if (!lwjgl::Mouse::isButtonDown(0))
			pressedColumn = -1;

		for (int_t column = 0; column < 3; ++column)
		{
			int_t columnX = x + 115 + column * 50 - 18;
			screen.drawStatsTexture(columnX, y + 1, 0, pressedColumn == column ? 0 : 18);
		}

		if (sortColumn != -1)
		{
			int_t arrowX = x + 79 + sortColumn * 50;
			screen.drawStatsTexture(arrowX, y + 1, sortDirection == 1 ? 36 : 18, 0);
		}

		drawColumnIcons(x, y);
	}

	void headerClicked(int_t x, int_t) override
	{
		pressedColumn = -1;
		if (x >= 79 && x < 115)
			pressedColumn = 0;
		else if (x >= 129 && x < 165)
			pressedColumn = 1;
		else if (x >= 179 && x < 215)
			pressedColumn = 2;

		if (pressedColumn >= 0)
		{
			sortBy(pressedColumn);
			screen.minecraft.soundEngine.playUI(u"random.click", 1.0f, 1.0f);
		}
	}

	void drawStatValue(StatBase *stat, int_t x, int_t y, bool light)
	{
		jstring value = stat == nullptr ? u"-" : stat->format(screen.stats.getStat(*stat));
		screen.drawString(screen.font, value, x - screen.font.width(value), y + 5, light ? 0xFFFFFF : 0x909090);
	}

	void drawOverlay(int_t mouseX, int_t mouseY) override
	{
		if (mouseY < top || mouseY > bottom)
			return;

		int_t index = getSlotIndex(mouseX, mouseY);
		int_t rowX = screen.width / 2 - 92 - 16;
		if (index >= 0)
		{
			if (mouseX < rowX + 40 || mouseX > rowX + 60)
				return;
			screen.drawTooltip(screen.getItemName(getEntry(index)->getItemId()), mouseX, mouseY);
			return;
		}

		int_t column = -1;
		if (mouseX >= rowX + 97 && mouseX <= rowX + 115)
			column = 0;
		else if (mouseX >= rowX + 147 && mouseX <= rowX + 165)
			column = 1;
		else if (mouseX >= rowX + 197 && mouseX <= rowX + 215)
			column = 2;
		if (column >= 0)
			screen.drawTooltip(StatCollector::translate(getColumnName(column)), mouseX, mouseY);
	}

	void sortBy(int_t column)
	{
		if (column != sortColumn)
		{
			sortColumn = column;
			sortDirection = -1;
		}
		else if (sortDirection == -1)
		{
			sortDirection = 1;
		}
		else
		{
			sortColumn = -1;
			sortDirection = 0;
		}

		std::sort(entries.begin(), entries.end(), [this](StatCrafting *first, StatCrafting *second) {
			int_t firstId = first->getItemId();
			int_t secondId = second->getItemId();
			StatBase *firstStat = sortColumn == -1 ? nullptr : getColumnStat(firstId, sortColumn);
			StatBase *secondStat = sortColumn == -1 ? nullptr : getColumnStat(secondId, sortColumn);
			if (firstStat != nullptr || secondStat != nullptr)
			{
				if (firstStat == nullptr)
					return false;
				if (secondStat == nullptr)
					return true;
				int_t firstValue = screen.stats.getStat(*firstStat);
				int_t secondValue = screen.stats.getStat(*secondStat);
				if (firstValue != secondValue)
					return javaStatComparison(firstValue, secondValue, sortDirection) < 0;
			}
			return firstId < secondId;
		});
	}

public:
	int_t entryCount() const
	{
		return static_cast<int_t>(entries.size());
	}
};

class BlockStatisticsList : public StatisticsList
{
protected:
	const char16_t *getColumnName(int_t column) const override
	{
		if (column == 0)
			return u"stat.crafted";
		return column == 1 ? u"stat.used" : u"stat.mined";
	}

	StatBase *getColumnStat(int_t itemId, int_t column) const override
	{
		if (column == 0)
			return StatList::craftItemStats[itemId];
		if (column == 1)
			return StatList::useItemStats[itemId];
		return StatList::mineBlockStats[itemId];
	}

	void drawColumnIcons(int_t x, int_t y) override
	{
		const int_t u[3] = {18, 36, 54};
		for (int_t column = 0; column < 3; ++column)
		{
			int_t columnX = x + 115 + column * 50 - 18;
			int_t offset = pressedColumn == column ? 1 : 0;
			screen.drawStatsTexture(columnX + offset, y + 1 + offset, u[column], 18);
		}
	}

	void drawSlot(int_t index, int_t x, int_t y, int_t, Tesselator &) override
	{
		StatCrafting *mined = getEntry(index);
		int_t itemId = mined->getItemId();
		screen.drawStatsItem(x + 40, y, itemId);
		drawStatValue(StatList::craftItemStats[itemId], x + 115, y, index % 2 == 0);
		drawStatValue(StatList::useItemStats[itemId], x + 165, y, index % 2 == 0);
		drawStatValue(mined, x + 215, y, index % 2 == 0);
	}

public:
	BlockStatisticsList(StatisticsScreen &screen) : StatisticsList(screen)
	{
		for (StatCrafting *mined : StatList::blockStats)
		{
			int_t itemId = mined->getItemId();
			if (screen.stats.getStat(*mined) > 0 ||
				(StatList::useItemStats[itemId] != nullptr && screen.stats.getStat(*StatList::useItemStats[itemId]) > 0) ||
				(StatList::craftItemStats[itemId] != nullptr && screen.stats.getStat(*StatList::craftItemStats[itemId]) > 0))
			{
				entries.push_back(mined);
			}
		}
	}
};

class ItemStatisticsList : public StatisticsList
{
protected:
	const char16_t *getColumnName(int_t column) const override
	{
		if (column == 1)
			return u"stat.crafted";
		return column == 2 ? u"stat.used" : u"stat.depleted";
	}

	StatBase *getColumnStat(int_t itemId, int_t column) const override
	{
		if (column == 0)
			return StatList::breakItemStats[itemId];
		if (column == 1)
			return StatList::craftItemStats[itemId];
		return StatList::useItemStats[itemId];
	}

	void drawColumnIcons(int_t x, int_t y) override
	{
		const int_t u[3] = {72, 18, 36};
		for (int_t column = 0; column < 3; ++column)
		{
			int_t columnX = x + 115 + column * 50 - 18;
			int_t offset = pressedColumn == column ? 1 : 0;
			screen.drawStatsTexture(columnX + offset, y + 1 + offset, u[column], 18);
		}
	}

	void drawSlot(int_t index, int_t x, int_t y, int_t, Tesselator &) override
	{
		StatCrafting *used = getEntry(index);
		int_t itemId = used->getItemId();
		screen.drawStatsItem(x + 40, y, itemId);
		drawStatValue(StatList::breakItemStats[itemId], x + 115, y, index % 2 == 0);
		drawStatValue(StatList::craftItemStats[itemId], x + 165, y, index % 2 == 0);
		drawStatValue(used, x + 215, y, index % 2 == 0);
	}

public:
	ItemStatisticsList(StatisticsScreen &screen) : StatisticsList(screen)
	{
		for (StatCrafting *used : StatList::itemStats)
		{
			int_t itemId = used->getItemId();
			if (screen.stats.getStat(*used) > 0 ||
				(StatList::breakItemStats[itemId] != nullptr && screen.stats.getStat(*StatList::breakItemStats[itemId]) > 0) ||
				(StatList::craftItemStats[itemId] != nullptr && screen.stats.getStat(*StatList::craftItemStats[itemId]) > 0))
			{
				entries.push_back(used);
			}
		}
	}
};

StatisticsScreen::StatisticsScreen(Minecraft &minecraft, std::shared_ptr<Screen> lastScreen, StatFileWriter &stats)
	: Screen(minecraft), lastScreen(std::move(lastScreen)), stats(stats)
{
}

StatisticsScreen::~StatisticsScreen() = default;

void StatisticsScreen::init()
{
	title = StatCollector::translate(u"gui.stats");
	generalList = std::make_unique<GeneralStatisticsList>(*this);
	generalList->registerScrollButtons(1, 1);
	itemList = std::make_unique<ItemStatisticsList>(*this);
	itemList->registerScrollButtons(1, 1);
	blockList = std::make_unique<BlockStatisticsList>(*this);
	blockList->registerScrollButtons(1, 1);
	activeList = generalList.get();
	addButtons();
}

void StatisticsScreen::addButtons()
{
	buttons.clear();
	buttons.push_back(Util::make_shared<Button>(0, width / 2 + 4, height - 28, 150, 20, StatCollector::translate(u"gui.done")));
	buttons.push_back(Util::make_shared<Button>(1, width / 2 - 154, height - 52, 100, 20, StatCollector::translate(u"stat.generalButton")));
	auto blocks = Util::make_shared<Button>(2, width / 2 - 46, height - 52, 100, 20, StatCollector::translate(u"stat.blocksButton"));
	buttons.push_back(blocks);
	auto items = Util::make_shared<Button>(3, width / 2 + 62, height - 52, 100, 20, StatCollector::translate(u"stat.itemsButton"));
	buttons.push_back(items);
	if (blockList->entryCount() == 0)
		blocks->active = false;
	if (itemList->entryCount() == 0)
		items->active = false;
}

void StatisticsScreen::buttonClicked(Button &button)
{
	if (!button.active)
		return;
	if (button.id == 0)
		minecraft.setScreen(lastScreen);
	else if (button.id == 1)
		activeList = generalList.get();
	else if (button.id == 3)
		activeList = itemList.get();
	else if (button.id == 2)
		activeList = blockList.get();
	else if (activeList != nullptr)
		activeList->actionPerformed(button);
}

void StatisticsScreen::render(int_t mouseX, int_t mouseY, float partialTick)
{
	if (activeList != nullptr)
		activeList->drawScreen(mouseX, mouseY, partialTick);
	drawCenteredString(font, title, width / 2, 20, 0xFFFFFF);
	Screen::render(mouseX, mouseY, partialTick);
}


void StatisticsScreen::drawStatsTexture(int_t x, int_t y, int_t u, int_t v)
{
	int_t texture = minecraft.textures.loadTexture(u"/gui/slot.png");
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	minecraft.textures.bind(texture);
	Tesselator &t = Tesselator::instance;
	t.begin();
	t.vertexUV(x, y + 18, blitOffset, u / 128.0f, (v + 18) / 128.0f);
	t.vertexUV(x + 18, y + 18, blitOffset, (u + 18) / 128.0f, (v + 18) / 128.0f);
	t.vertexUV(x + 18, y, blitOffset, (u + 18) / 128.0f, v / 128.0f);
	t.vertexUV(x, y, blitOffset, u / 128.0f, v / 128.0f);
	t.end();
}


void StatisticsScreen::drawStatsItem(int_t x, int_t y, int_t itemId)
{
	drawStatsTexture(x + 1, y + 1, 0, 0);
	glEnable(GL_RESCALE_NORMAL);
	glPushMatrix();
	glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
	Lighting::turnOn();
	glPopMatrix();
	static ItemRenderer itemRenderer(EntityRenderDispatcher::instance);
	ItemInstance stack(itemId, 1, 0);
	itemRenderer.renderGuiItem(font, minecraft.textures, stack, x + 2, y + 2);
	Lighting::turnOff();
	glDisable(GL_RESCALE_NORMAL);
}

void StatisticsScreen::drawTooltip(const jstring &text, int_t x, int_t y)
{
	if (text.empty())
		return;
	int_t tooltipX = x + 12;
	int_t tooltipY = y - 12;
	int_t tooltipWidth = font.width(text);
	fillGradient(tooltipX - 3, tooltipY - 3, tooltipX + tooltipWidth + 3, tooltipY + 11, 0xC0000000, 0xC0000000);
	font.drawShadow(text, tooltipX, tooltipY, 0xFFFFFF);
}

jstring StatisticsScreen::getItemName(int_t itemId) const
{
	jstring key;
	if (itemId >= 0 && itemId < static_cast<int_t>(Tile::tiles.size()) && Tile::tiles[itemId] != nullptr)
		key = Tile::tiles[itemId]->descriptionId;
	else if (itemId >= 0 && itemId < static_cast<int_t>(Item::items.size()) && Item::items[itemId] != nullptr)
		key = Item::items[itemId]->getDescriptionId();
	return trimJava(Language::getInstance().getElementName(key));
}
