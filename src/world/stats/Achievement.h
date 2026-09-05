#pragma once

#include <functional>

#include "world/item/ItemInstance.h"
#include "world/stats/StatBase.h"

class Achievement : public StatBase
{
private:
	jstring achievementDescription;
	std::function<jstring(const jstring &)> descriptionFormatter;
	bool special = false;

public:
	const int_t displayColumn;
	const int_t displayRow;
	Achievement *const parentAchievement;
	const ItemInstance item;

	Achievement(int_t id, const jstring &name, int_t column, int_t row, const ItemInstance &item, Achievement *parent);

	Achievement &markIndependentAchievement();
	Achievement &setSpecial();
	Achievement &registerAchievement();
	Achievement &setDescriptionFormatter(std::function<jstring(const jstring &)> formatter);

	bool isAchievement() const override;
	jstring getDescription() const;
	bool isSpecial() const;
};
