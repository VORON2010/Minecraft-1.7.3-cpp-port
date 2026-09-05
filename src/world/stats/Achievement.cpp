#include "world/stats/Achievement.h"

#include "world/stats/AchievementList.h"
#include "world/stats/StatCollector.h"

Achievement::Achievement(int_t id, const jstring &name, int_t column, int_t row, const ItemInstance &item, Achievement *parent)
	: StatBase(5242880 + id, StatCollector::translate(u"achievement." + name)),
	  achievementDescription(StatCollector::translate(u"achievement." + name + u".desc")),
	  displayColumn(column), displayRow(row), parentAchievement(parent), item(item)
{
	if (column < AchievementList::minDisplayColumn)
		AchievementList::minDisplayColumn = column;
	if (row < AchievementList::minDisplayRow)
		AchievementList::minDisplayRow = row;
	if (column > AchievementList::maxDisplayColumn)
		AchievementList::maxDisplayColumn = column;
	if (row > AchievementList::maxDisplayRow)
		AchievementList::maxDisplayRow = row;
}

Achievement &Achievement::markIndependentAchievement()
{
	independent = true;
	return *this;
}

Achievement &Achievement::setSpecial()
{
	special = true;
	return *this;
}

Achievement &Achievement::registerAchievement()
{
	StatBase::registerStat();
	AchievementList::achievements.push_back(this);
	return *this;
}

Achievement &Achievement::setDescriptionFormatter(std::function<jstring(const jstring &)> formatter)
{
	descriptionFormatter = std::move(formatter);
	return *this;
}

bool Achievement::isAchievement() const
{
	return true;
}

jstring Achievement::getDescription() const
{
	return descriptionFormatter ? descriptionFormatter(achievementDescription) : achievementDescription;
}

bool Achievement::isSpecial() const
{
	return special;
}
