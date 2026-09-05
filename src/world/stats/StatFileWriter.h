#pragma once

#include <memory>
#include <unordered_map>

#include "java/String.h"
#include "java/Type.h"

class Achievement;
class File;
class StatBase;
class StatsSyncher;
class User;

using StatMap = std::unordered_map<const StatBase *, int_t>;

class StatFileWriter
{
private:
	StatMap totalStats;
	StatMap sessionStats;
	bool dirty = false;
	std::unique_ptr<StatsSyncher> statsSyncher;

	static void addToMap(StatMap &map, const StatBase &stat, int_t amount);

public:
	StatFileWriter(const User &user, const File &directory);
	~StatFileWriter();

	void readStat(const StatBase &stat, int_t amount);
	StatMap copySessionStats() const;
	void mergeUnsent(const StatMap &stats);
	void mergeLoadedTotal(const StatMap &stats);
	void mergeSessionOnly(const StatMap &stats);

	static std::unique_ptr<StatMap> parse(const std::string &json);
	static std::string serialize(const jstring *username, const jstring *sessionId, const StatMap &stats);

	bool hasAchievementUnlocked(const Achievement &achievement) const;
	bool canUnlockAchievement(const Achievement &achievement) const;
	int_t getStat(const StatBase &stat) const;

	void syncStats();
	void tick();
};
