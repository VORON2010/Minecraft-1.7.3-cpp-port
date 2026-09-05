#include "client/gui/WorldSelectionList.h"

#include <cstdio>
#include <ctime>

#include "client/Minecraft.h"
#include "client/gui/SelectWorldScreen.h"
#include "client/locale/Language.h"

namespace
{


jstring formatDate(long_t lastPlayed)
{
	std::time_t time = static_cast<std::time_t>(lastPlayed / 1000LL);
	std::tm tm = {};
#ifdef _WIN32
	localtime_s(&tm, &time);
#else
	localtime_r(&time, &tm);
#endif
	int hour12 = tm.tm_hour % 12;
	if (hour12 == 0)
		hour12 = 12;
	char buffer[64] = {};
	std::snprintf(buffer, sizeof(buffer), "%d/%d/%02d %d:%02d %s", tm.tm_mon + 1, tm.tm_mday, tm.tm_year % 100, hour12, tm.tm_min, tm.tm_hour < 12 ? "AM" : "PM");
	return String::fromUTF8(buffer);
}


jstring formatSize(long_t sizeOnDisk)
{
	long_t hundredths = sizeOnDisk / 1024LL * 100LL / 1024LL;
	return String::toString(static_cast<float>(hundredths) / 100.0f) + u" MB";
}

}

WorldSelectionList::WorldSelectionList(Minecraft &minecraft, SelectWorldScreen &screen)
	: GuiSlot(minecraft, screen.width, screen.height, 32, screen.height - 64, 36), screen(screen)
{

}

int_t WorldSelectionList::getSize()
{
	return static_cast<int_t>(screen.getWorlds().size());
}

void WorldSelectionList::elementClicked(int_t index, bool doubleClick)
{
	screen.setSelectedWorld(index);
	screen.updateSelectionButtons();
	if (doubleClick && screen.getSummary(index) != nullptr)
		screen.selectWorld(index);
}

bool WorldSelectionList::isSelected(int_t index)
{
	return index == screen.getSelectedWorld();
}

int_t WorldSelectionList::getContentHeight()
{
	return getSize() * 36;
}

void WorldSelectionList::drawBackground()
{
	screen.renderBackground();
}

void WorldSelectionList::drawSlot(int_t index, int_t x, int_t y, int_t height, Tesselator &t)
{
	const Level::Summary *summary = screen.getSummary(index);
	if (summary == nullptr)
		return;

	jstring title = summary->levelName.empty() ? summary->folderName : summary->levelName;
	jstring details = summary->folderName + u" (" + formatDate(summary->lastPlayed) + u", " + formatSize(summary->sizeOnDisk) + u")";
	jstring warning = summary->requiresConversion ? Language::getInstance().getElement(u"selectWorld.conversion") : u"";
	Font &font = *minecraft.font;

	screen.drawString(font, title, x + 2, y + 1, 0xFFFFFF);
	screen.drawString(font, details, x + 2, y + 12, 0x808080);
	if (!warning.empty())
		screen.drawString(font, warning, x + 2, y + 22, 0x808080);
}
