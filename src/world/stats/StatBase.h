#pragma once

#include <memory>

#include "java/String.h"
#include "java/Type.h"

class StatFormatter
{
public:
	virtual ~StatFormatter() = default;
	virtual jstring format(int_t value) const = 0;
};

class StatBase
{
private:
	const StatFormatter &formatter;

public:
	const int_t statId;
	const jstring statName;
	bool independent = false;
	jstring statGuid;

	StatBase(int_t id, const jstring &name);
	StatBase(int_t id, const jstring &name, const StatFormatter &formatter);
	virtual ~StatBase() = default;

	StatBase &markIndependent();
	virtual StatBase &registerStat();
	virtual bool isAchievement() const;
	jstring format(int_t value) const;

	static const StatFormatter &simpleFormatter();
	static const StatFormatter &timeFormatter();
	static const StatFormatter &distanceFormatter();
};
