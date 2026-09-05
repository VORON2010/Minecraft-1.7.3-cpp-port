#include "world/stats/StatBase.h"

#include <iomanip>
#include <sstream>

#include "world/stats/AchievementMap.h"
#include "world/stats/StatList.h"

namespace
{
	jstring formatDecimal(double value)
	{
		std::ostringstream stream;
		stream.imbue(std::locale::classic());
		stream << std::fixed << std::setprecision(2) << value;
		return String::fromUTF8(stream.str());
	}

	jstring formatJavaSeconds(int_t ticks)
	{
		long_t signedTicks = ticks;
		bool negative = signedTicks < 0;
		ulong_t hundredths = static_cast<ulong_t>(negative ? -signedTicks : signedTicks) * 5;
		ulong_t whole = hundredths / 100;
		unsigned int fraction = static_cast<unsigned int>(hundredths % 100);
		std::string wholeText = std::to_string(whole);
		std::string result;

		if (whole >= 10000000)
		{
			std::string digits = wholeText;
			digits.push_back(static_cast<char>('0' + fraction / 10));
			digits.push_back(static_cast<char>('0' + fraction % 10));
			while (digits.size() > 1 && digits.back() == '0')
				digits.pop_back();

			result = digits.substr(0, 1) + ".";
			result += digits.size() == 1 ? "0" : digits.substr(1);
			result += "E" + std::to_string(wholeText.size() - 1);
		}
		else
		{
			result = wholeText + ".";
			if (fraction == 0)
				result += "0";
			else
			{
				if (fraction < 10)
					result += "0";
				result += std::to_string(fraction);
				if (result.back() == '0')
					result.pop_back();
			}
		}

		if (negative)
			result.insert(result.begin(), '-');
		return String::fromUTF8(result);
	}

	class SimpleStatFormatter : public StatFormatter
	{
	public:
		jstring format(int_t value) const override
		{
			std::string text = std::to_string(value);
			size_t start = !text.empty() && text[0] == '-' ? 1 : 0;
			for (size_t i = text.size(); i > start + 3; i -= 3)
				text.insert(i - 3, 1, ',');
			return String::fromUTF8(text);
		}
	};

	class TimeStatFormatter : public StatFormatter
	{
	public:
		jstring format(int_t value) const override
		{
			double seconds = value / 20.0;
			double minutes = seconds / 60.0;
			double hours = minutes / 60.0;
			double days = hours / 24.0;
			double years = days / 365.0;
			if (years > 0.5)
				return formatDecimal(years) + u" y";
			if (days > 0.5)
				return formatDecimal(days) + u" d";
			if (hours > 0.5)
				return formatDecimal(hours) + u" h";
			if (minutes > 0.5)
				return formatDecimal(minutes) + u" m";

			return formatJavaSeconds(value) + u" s";
		}
	};

	class DistanceStatFormatter : public StatFormatter
	{
	public:
		jstring format(int_t value) const override
		{
			double metres = value / 100.0;
			double kilometres = metres / 1000.0;
			if (kilometres > 0.5)
				return formatDecimal(kilometres) + u" km";
			if (metres > 0.5)
				return formatDecimal(metres) + u" m";
			return String::toString(value) + u" cm";
		}
	};

	SimpleStatFormatter simpleFormatterInstance;
	TimeStatFormatter timeFormatterInstance;
	DistanceStatFormatter distanceFormatterInstance;
}

StatBase::StatBase(int_t id, const jstring &name)
	: StatBase(id, name, simpleFormatter())
{
}

StatBase::StatBase(int_t id, const jstring &name, const StatFormatter &formatter)
	: formatter(formatter), statId(id), statName(name)
{
}

StatBase &StatBase::markIndependent()
{
	independent = true;
	return *this;
}

StatBase &StatBase::registerStat()
{
	StatList::registerStat(*this);
	statGuid = AchievementMap::getGuid(statId);
	return *this;
}

bool StatBase::isAchievement() const
{
	return false;
}

jstring StatBase::format(int_t value) const
{
	return formatter.format(value);
}

const StatFormatter &StatBase::simpleFormatter()
{
	return simpleFormatterInstance;
}

const StatFormatter &StatBase::timeFormatter()
{
	return timeFormatterInstance;
}

const StatFormatter &StatBase::distanceFormatter()
{
	return distanceFormatterInstance;
}
